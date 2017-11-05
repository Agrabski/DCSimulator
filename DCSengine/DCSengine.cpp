#include "pch.h"
#include "DCSengine.h"
#include <cmath>

void DCS::Game::gameTick()
{
	x += (targetX - x) > 0 ? 1 : ((targetX - x) == 0 ? 0 : -1);
	y += (targetY - y) > 0 ? 1 : ((targetY - y) == 0 ? 0 : -1);
}

DCS::Ship::Ship()
{
	silvete.emplace_back(Point(0, 0));
	silvete.emplace_back(Point(100, 0));
	silvete.emplace_back(Point(100, 300));
	silvete.emplace_back(Point(0, 300));
	std::vector<Point>bridgeSilvete;
	bridgeSilvete.emplace_back(Point(5, 5));
	bridgeSilvete.emplace_back(Point(40, 0));
	bridgeSilvete.emplace_back(Point(40, 80));
	bridgeSilvete.emplace_back(Point(5, 80));
	std::vector<StaticEntity> e;
	rooms.emplace_back(new Room(Point(10, 10), bridgeSilvete, e, Room::RoomType::Bridge));
}

void DCS::Room::update()
{
	throw std::runtime_error("not implemented");
}

DCS::Room::Room(Point position, std::vector<Point> Silvete, std::vector<StaticEntity> entities, RoomType type)
{
	this->position = position;
	this->silvete = Silvete;
	staticEntities = entities;
	this->type = type;
}

DCS::Point DCS::operator+(const Point & left, const Point & right)
{
	return Point(left.first + right.first, left.second + right.second);
}

DCS::Room * DCS::MobileEntity::update()
{
	throw std::runtime_error("not implemented");

}

DCS::Room * DCS::StaticEntity::update()
{
	throw std::runtime_error("not implemented");
}
