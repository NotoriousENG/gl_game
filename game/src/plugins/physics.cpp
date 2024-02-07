#include "plugins/physics.hpp"

void push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                         Transform2D &t1, CollisionVolume &c1) {
  const auto rect1Center = glm::vec2(rect.x + rect.w / 2, rect.y + rect.h / 2);

  const auto rect2Center =
      glm::vec2(pushedBy.x + pushedBy.w / 2, pushedBy.y + pushedBy.h / 2);

  const auto distanceX = rect1Center.x - rect2Center.x;
  const auto distanceY = rect1Center.y - rect2Center.y;

  // Figure out the combined half-widths and half-heights
  const auto halfWidths = (rect.w + pushedBy.w) / 2;
  const auto halfHeights = (rect.h + pushedBy.h) / 2;

  // Calculate the overlap between the two rectangles
  const auto overlapX = halfWidths - std::abs(distanceX);
  const auto overlapY = halfHeights - std::abs(distanceY);

  // The collision has happened on the axis of least penetration
  const bool isMoreVertical = overlapX > overlapY;

  if (isMoreVertical) {
    if (distanceY < 0) {
      t1.position.y = pushedBy.y - rect.h - c1.vertices.y;
    } else {
      t1.position.y = pushedBy.y + pushedBy.h - c1.vertices.y;
    }
  } else {
    if (distanceX < 0) {
      t1.position.x = pushedBy.x - rect.w - c1.vertices.x;
    } else {
      t1.position.x = pushedBy.x + pushedBy.w - c1.vertices.x;
    }
  }
}

void applyGravity(flecs::iter it, Velocity *v, Groundable *g) {
  const auto dt = it.delta_time();
  const float grav = it.world().get<Gravity>()->value;
  for (int i : it) {
    if (g[i].isGrounded) {
      v[i].value.y = 0.0f;
    } else {
      v[i].value.y += grav * dt;
    }
  }
}

void applyVelocity(flecs::iter it, Velocity *v, Transform2D *t) {
  const auto dt = it.delta_time();
  for (int i : it) {
    t[i].position += v[i].value * dt;
    t[i].global_position = t[i].position;
  }
}

void runEntityCollisions(flecs::entity e1, Transform2D &t1, CollisionVolume &c1,
                         flecs::entity e2, Transform2D &t2,
                         CollisionVolume &c2) {
  if (e1.id() == e2.id()) {
    return;
  }
  const SDL_Rect rect1 = {
      static_cast<int>(t1.global_position.x + c1.vertices.x),
      static_cast<int>(t1.global_position.y + c1.vertices.y),
      static_cast<int>(c1.vertices.z - c1.vertices.x),
      static_cast<int>(c1.vertices.w - c1.vertices.y)};
  const SDL_Rect rect2 = {
      static_cast<int>(t2.global_position.x + c2.vertices.x),
      static_cast<int>(t2.global_position.y + c2.vertices.y),
      static_cast<int>(c2.vertices.z - c2.vertices.x),
      static_cast<int>(c2.vertices.w - c2.vertices.y)};

  if (SDL_HasIntersection(&rect1, &rect2)) {
    // apply damage
    if (e1.has<Hurtbox>() && e2.has<Health>() && e1.get<Hurtbox>()->active) {
      e2.get_mut<Health>()->value -= e1.get<Hurtbox>()->damage;
      if (e2.get<Health>()->value <= 0) {
        e2.destruct();
      }
    }
    // push entities apart
    if (e1.has<PhysicsBody>() && e2.has<StaticBody>()) {
      push_rect_transform(rect1, rect2, t1, c1);
    }
  }
}

void dieOfOldAge(flecs::iter it, LiveFor *l) {
  const auto dt = it.delta_time();
  for (int i : it) {
    l[i].seconds -= dt;
    if (l[i].seconds <= 0) {
      it.entity(i).destruct();
    }
  }
}

void PhysicsPlugin::addSystems(flecs::world &ecs) {
  // gravity system
  ecs.system<Velocity, Groundable>().iter(
      [](flecs::iter it, Velocity *v, Groundable *g) {
        applyGravity(it, v, g);
      });

  // velocity system
  ecs.system<Velocity, Transform2D>().iter(
      [](flecs::iter it, Velocity *v, Transform2D *t) {
        applyVelocity(it, v, t);
      });

  const auto collisionQuery = ecs.query<Transform2D, CollisionVolume>();
  ecs.system<Transform2D, CollisionVolume>().each(
      [collisionQuery](flecs::entity e1, Transform2D &t1, CollisionVolume &c1) {
        collisionQuery.each([&e1, &t1, &c1](flecs::entity e2, Transform2D &t2,
                                            CollisionVolume &c2) {
          runEntityCollisions(e1, t1, c1, e2, t2, c2);
        });
      });

  // die of old age system
  ecs.system<LiveFor>().iter(
      [](flecs::iter it, LiveFor *l) { dieOfOldAge(it, l); });
}
