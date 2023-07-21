#include "rtc/rtc.hpp"
#include <functional>
#include <string>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
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


  void createDataChannel(shared_ptr<rtc::PeerConnection> pc, std::string id) {
    // We are the offerer, so create a data channel to initiate the process
    const std::string label = "test";
    std::cout << "Creating DataChannel with label \"" << label << "\""
              << std::endl;
    auto dc = pc->createDataChannel(label);

    setupDataChannel(dc, id);

    dataChannels.emplace(id, dc);
  }

  void setupDataChannel(std::shared_ptr<rtc::DataChannel> dc, std::string id) {
    dc->onOpen([this, id, wdc = make_weak_ptr(dc)]() {
      std::cout << "DataChannel from " << id << " open" << std::endl;
      onConnection(id);
    });

    dc->onClosed([id]() {
      std::cout << "DataChannel from " << id << " closed" << std::endl;
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

    pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "State: " << state << std::endl;
    });

    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
      std::cout << "Gathering State: " << state << std::endl;
    });

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
      std::cout << "DataChannel from " << id << " received with label \""
                << dc->label() << "\"" << std::endl;
      setupDataChannel(dc, id);
      dataChannels.emplace(id, dc);
    });

    peerConnections.emplace(id, pc);
    return pc;
  };

public:
  NetManager(std::function<void(std::string)> onConnection,
             std::function<void(std::string, std::string)> onMessage)
      : onConnection(onConnection), onMessage(onMessage) {}

  ~NetManager() {
    dataChannels.clear();
    peerConnections.clear();
  }

  void connectToSignaling() {
    ws = std::make_shared<rtc::WebSocket>();
    std::promise<void> wsPromise;
    auto wsFuture = wsPromise.get_future();

    ws->onOpen([&wsPromise]() {
      std::cout << "WebSocket connected, signaling ready" << std::endl;
      wsPromise.set_value();
    });

    ws->onError([&wsPromise](std::string s) {
      std::cout << "WebSocket error" << std::endl;
      wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    ws->onClosed([]() { std::cout << "WebSocket closed" << std::endl; });

    ws->onMessage([this, wws = make_weak_ptr(ws)](auto data) {
      std::cout << "Got a message" << std::endl;

      // data holds either std::string or rtc::binary; we require a string
      if (!std::holds_alternative<std::string>(data)) {
        std::cout << "message not a string" << std::endl;
        return;
      }

      std::cout << "Message is: " << std::get<std::string>(data) << std::endl;

      json message = json::parse(std::get<std::string>(data));

      // always an id of the sender
      auto it = message.find("id");
      if (it == message.end()) {
        std::cout << "could not find id" << std::endl;
        return;
      }
      auto id = it->get<std::string>();

      std::cout << "found id: " << id << std::endl;

      // and a type
      it = message.find("type");
      if (it == message.end()) {
        std::cout << "could not find type" << std::endl;
        return;
      }
      auto type = it->get<std::string>();

      std::cout << "found type: " << type << std::endl;

      // get the peer connection based on the id
      // or make it if it doesn't yet exist
      shared_ptr<rtc::PeerConnection> pc;
      if (auto jt = peerConnections.find(id); jt != peerConnections.end()) {
        pc = jt->second;
      } else if (type == "offer" || type == "requestOffer") {
        pc = createPeerConnection(config, wws, id);
        std::cout << "making new peer connection" << std::endl;
      } else {
        return;
      }

      // finally catch up on the actual message
      if (type == "requestOffer") {
        createDataChannel(pc, id);
        std::cout << "making new data channel" << std::endl;
      } else if (type == "offer" || type == "answer") {
        auto sdp = message["description"].get<std::string>();
        pc->setRemoteDescription(rtc::Description(sdp, type));
        std::cout << "getting response to offer/answer" << std::endl;
      } else if (type == "candidate") {
        auto sdp = message["candidate"].get<std::string>();
        auto mid = message["mid"].get<std::string>();
        pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
        std::cout << "dealing with the candidate" << std::endl;
      }
    });

    ws->open("ws://localhost:8000"); // is this the right port??

    std::cout << "Waiting for signaling to be connected..." << std::endl;
    wsFuture.get();
  }

  void sendTo(std::string id, std::string message) {
    if (auto jt = dataChannels.find(id); jt != dataChannels.end()) {
      jt->second->send(message);
    }
  }
};