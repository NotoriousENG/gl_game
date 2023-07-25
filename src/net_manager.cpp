#include "rtc/rtc.hpp"
#include <functional>
#include <string>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <future>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>

using namespace std::chrono_literals;
using nlohmann::json;
using std::shared_ptr;
using std::weak_ptr;
template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

class NetManager {
private:
  std::function<void(std::string)> onConnection;
  std::function<void(std::string, std::string)> onMessage;
  std::unordered_map<std::string, shared_ptr<rtc::PeerConnection>>
      peerConnections;
  std::unordered_map<std::string, shared_ptr<rtc::DataChannel>> dataChannels;
  std::shared_ptr<rtc::WebSocket> ws;
  rtc::Configuration config;
  std::string hostJoin;
  std::string roomCode;

  void createDataChannel(shared_ptr<rtc::PeerConnection> pc, std::string id) {
    // We are the offerer, so create a data channel to initiate the process
    const std::string label = "test";
    printf("creating datachannel\n");
    auto dc = pc->createDataChannel(label);

    setupDataChannel(dc, id);

    dataChannels.emplace(id, dc);
  }

  void setupDataChannel(std::shared_ptr<rtc::DataChannel> dc, std::string id) {
    dc->onOpen([this, id, wdc = make_weak_ptr(dc)]() {
      printf("opened channel with %s\n", id.c_str());
      onConnection(id);
    });

    dc->onClosed([id]() { printf("creating datachannel\n");
      printf("closed channel with %s\n", id.c_str());
    });

    dc->onMessage([this, id, wdc = make_weak_ptr(dc)](auto data) {
      // data holds either std::string or rtc::binary; we need string
      if (std::holds_alternative<std::string>(data)) {
        onMessage(id, std::get<std::string>(data));
      } else {
        return;
      }
    });
  }

  shared_ptr<rtc::PeerConnection>
  createPeerConnection(const rtc::Configuration &config,
                       weak_ptr<rtc::WebSocket> wws, std::string id) {
    auto pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onLocalDescription([wws, id](rtc::Description description) {
      json message = {{"id", id},
                      {"type", description.typeString()},
                      {"description", std::string(description)}};

      if (auto ws = wws.lock()) {
        ws->send(message.dump());
      }
    });

    pc->onLocalCandidate([wws, id](rtc::Candidate candidate) {
      json message = {{"id", id},
                      {"type", "candidate"},
                      {"candidate", std::string(candidate)},
                      {"mid", candidate.mid()}};

      if (auto ws = wws.lock()) {
        ws->send(message.dump());
      }
    });

    pc->onDataChannel([this, id](shared_ptr<rtc::DataChannel> dc) {
      printf("recieved channel from %s with label %s\n", id.c_str(), dc->label().c_str());
      setupDataChannel(dc, id);
      dataChannels.emplace(id, dc);
    });

    peerConnections.emplace(id, pc);
    return pc;
  };

public:
  NetManager(std::string hostJoin, std::string roomCode,
             std::function<void(std::string)> onConnection,
             std::function<void(std::string, std::string)> onMessage)
      : hostJoin(hostJoin), roomCode(roomCode), onConnection(onConnection),
        onMessage(onMessage) {
    config.iceServers.emplace_back("stun:stun1.l.google.com:19302");
    config.iceServers.emplace_back("stun:stun2.l.google.com:19302");
    config.iceServers.emplace_back("stun:stun3.l.google.com:19302");
    config.iceServers.emplace_back("stun:stun4.l.google.com:19302");
  }

  ~NetManager() {
    dataChannels.clear();
    peerConnections.clear();
  }

  void connectToSignaling() {
    ws = std::make_shared<rtc::WebSocket>();

    ws->onOpen([this, wws = make_weak_ptr(ws)]() {
      printf("wsopen");
      if (auto ws = wws.lock()) {
        if (hostJoin == "host") {
          json message = {{"type", "host"}};
          ws->send(message.dump());
        } else if (hostJoin == "join") {
          json message = {{"type", "join"}, {"roomCode", roomCode}};
          ws->send(message.dump());
        } else {
        }
      }
    });

    ws->onError([](std::string s) {
      printf("wserror");
    });

    ws->onClosed([]() { printf("wsclosed"); });

    ws->onMessage([this, wws = make_weak_ptr(ws)](auto data) {
      // data holds either std::string or rtc::binary; we require a string
      if (!std::holds_alternative<std::string>(data)) {
        return;
      }

      auto messageText = std::get<std::string>(data);

      printf("got ws message: %s\n", messageText.c_str());

      json message = json::parse(messageText);

      // always a type
      auto it = message.find("type");
      if (it == message.end()) {
        return;
      }
      auto type = it->get<std::string>();

      if (type == "roomCode") {
        it = message.find("roomCode");
        if (it == message.end()) {
          return;
        }
        auto code = it->get<std::string>();
        printf("assigned room code: %s", code.c_str());
        return;
      } else if (type == "exception") {
        it = message.find("message");
        if (it == message.end()) {
          return;
        }
        auto exceptionMessage = it->get<std::string>();
        it = message.find("code");
        if (it == message.end()) {
          return;
        }
        auto code = it->get<std::string>();
        return;
      }

      // other ones all have an id
      it = message.find("id");
      if (it == message.end()) {
        return;
      }
      auto id = it->get<std::string>();


      // get the peer connection based on the id
      // or make it if it doesn't yet exist
      shared_ptr<rtc::PeerConnection> pc;
      if (auto jt = peerConnections.find(id); jt != peerConnections.end()) {
        pc = jt->second;
      } else if (type == "offer" || type == "requestOffer") {
        pc = createPeerConnection(config, wws, id);
      } else {
        return;
      }

      // finally catch up on the actual message
      if (type == "requestOffer") {
        createDataChannel(pc, id);
      } else if (type == "offer" || type == "answer") {
        auto sdp = message["description"].get<std::string>();
        pc->setRemoteDescription(rtc::Description(sdp, type));
      } else if (type == "candidate") {
        auto sdp = message["candidate"].get<std::string>();
        auto mid = message["mid"].get<std::string>();
        pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
      }
    });

    ws->open("wss://gl-game-backend.deno.dev");
    printf("swag.\n");
  }

  void sendTo(std::string id, std::string message) {
    if (auto jt = dataChannels.find(id); jt != dataChannels.end()) {
      jt->second->send(message);
    }
  }
};