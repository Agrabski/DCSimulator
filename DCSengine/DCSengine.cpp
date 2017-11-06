#include "pch.h"
#include "DCSengine.h"
#include <cmath>

void DCS::Game::gameTick()
{
	for (std::vector<MobileEntity*>::iterator i = ship.mobileEntities.begin(); i != ship.mobileEntities.end(); i++)
	{
		Room* tmp = (*i)->location();
		if (tmp != (*i)->update())
		{
			tmp->removeEntity(*i);
			(*i)->location()->addEntity(*i);
		}
	}
	for (std::vector<Room*>::iterator i = ship.rooms.begin(); i != ship.rooms.end(); i++)
		(*i)->update();
}


bool pnpoly(std::vector<DCS::Point>vert, float testx, float testy)
{
	int i, j;
	bool c = false;
	for (i = 0, j = vert.size() - 1; i < vert.size(); j = i++) {
		if (((vert[i].second>testy) != ((vert[j].second>testy)) &&
			(testx < (vert[j].first - vert[i].first) * (testy - vert[i].second) / (vert[j].second - vert[i].second) + vert[i].first)))
			c = !c;
	}
	return c;
}


DCS::Room * DCS::Ship::findRoom(Point p)
{
	for (std::vector<Room*>::iterator i = rooms.begin(); i != rooms.end(); i++)
		if (pnpoly((*i)->silvete, p.first, p.second))
			return *i;
	return nullptr;
}

DCS::Ship::Ship()
{
	silvete.emplace_back(Point(0, 0));
	silvete.emplace_back(Point(300, 0));
	silvete.emplace_back(Point(300, 1000));
	silvete.emplace_back(Point(0, 1000));

	std::vector<StaticEntity> e;
	std::vector<Point>bridgeSilvete;
	bridgeSilvete.emplace_back(Point(0, 0));
	bridgeSilvete.emplace_back(Point(40, 0));
	bridgeSilvete.emplace_back(Point(40, 85));
	bridgeSilvete.emplace_back(Point(0, 85));
	Room* bridge = new Room(Point(10, 10), bridgeSilvete, e, Room::RoomType::Bridge);

	std::vector<Point>corridorSilvete;
	corridorSilvete.emplace_back(Point(0, 0));
	corridorSilvete.emplace_back(Point(20, 0));
	corridorSilvete.emplace_back(Point(20, 10));
	corridorSilvete.emplace_back(Point(0, 10));
	Room*corridor = new Room(Point(50, 35), corridorSilvete, e, Room::Corridor);

	MobileEntity*entity = new MobileEntity(bridge, Point(10, 10), MobileEntity::Engineer);
	bridge->setRight(corridor, corridor->position+Point(0,5));
	bridge->addEntity(entity);
	rooms.push_back(bridge);
	rooms.push_back(corridor);
	mobileEntities.push_back(entity);
}

void DCS::Room::update()
{
	//throw std::runtime_error("not implemented");
}

DCS::Room::Room(Point position, std::vector<Point> Silvete, std::vector<StaticEntity> entities, RoomType type)
{
	this->position = position;
	this->silvete = Silvete;
	staticEntities = entities;
	this->type = type;
}

void DCS::Room::setUp(Room *room, Point position)
{
	if (this->up == nullptr)
	{
		up = room;
		room->setDown(this, this->position);
		positionUp = position;
	}
}

void DCS::Room::setDown(Room *room, Point position)
{
	if (this->down == nullptr)
	{
		down = room;
		room->setUp(this, this->position);
		positionDown = position;
	}
}

void DCS::Room::setLeft(Room *room, Point position)
{
	if (this->left == nullptr)
	{
		left = room;
		room->setRight(this, this->position);
		positionLeft = position;
	}
}

void DCS::Room::setRight(Room *room, Point position)
{
	if (this->right == nullptr)
	{
		right = room;
		room->setLeft(this, this->position);
		positionRight = position;
	}
}

void DCS::Room::removeEntity(MobileEntity * e)
{
	for (std::vector<MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++)
	{
		if (*i == e)
		{
			mobileEntities.erase(i);
			return;
		}
	}
}

void DCS::Room::addEntity(MobileEntity * e)
{
	mobileEntities.push_back(e);
}

std::pair<DCS::Room*, DCS::Point> DCS::Room::leftDoor()
{
	return std::pair<Room*, Point>(left,positionLeft);
}

std::pair<DCS::Room*, DCS::Point> DCS::Room::rightDoor()
{
	return std::pair<Room*, Point>(right,positionRight);
}

std::pair<DCS::Room*, DCS::Point> DCS::Room::downDoor()
{
	return std::pair<Room*, Point>(down,positionDown);
}

std::pair<DCS::Room*, DCS::Point> DCS::Room::upDoor()
{
	return std::pair<Room*, Point>(up,positionUp);
}

DCS::Point DCS::operator+(const Point & left, const Point & right)
{
	return Point(left.first + right.first, left.second + right.second);
}

DCS::Point DCS::operator-(const Point & left, const Point & right)
{
	return Point(left.first - right.first, left.second - right.second);
}

bool DCS::operator<(const Point & left, const Point & right)
{
	return sqrt(left.first*left.first + left.second*left.second) < sqrt(right.first*right.first + right.second*right.second);
}

float DCS::magnitude(const Point & op)
{
	return sqrt(op.first*op.first + op.second*op.second);
}

DCS::Point DCS::MobileEntity::findDoor(Room * next)
{
	std::pair<Room*, Point> k;
	if ((k = currentRoom->leftDoor()).first == next)
		return k.second;
	if ((k = currentRoom->rightDoor()).first == next)
		return k.second;
	if ((k = currentRoom->downDoor()).first == next)
		return k.second;
	if ((k = currentRoom->upDoor()).first == next)
		return k.second;
}

void DCS::MobileEntity::findPath()
{
}

DCS::Room * DCS::MobileEntity::update()
{
	if (destination.second == nullptr)
		return currentRoom;	
	if (destination.second == currentRoom)
	{
		position = position + DCS::Point((destination.first.first - position.first) > 0 ? 1 : ((destination.first.first - position.first) == 0 ? 0 : -1), (destination.first.second - position.second) > 0 ? 1 : ((destination.first.second - position.second) == 0 ? 0 : -1));
	}
	if (path.size() != 0 && (destination.second == path[path.size() - 1]))
	{
		Point tmp = findDoor(path[0]);
		position = position + DCS::Point((tmp.first - position.first) > 0 ? 1 : ((tmp.first - position.first) == 0 ? 0 : -1), (tmp.second - position.second) > 0 ? 1 : ((tmp.second - position.second) == 0 ? 0 : -1));
		if (position == tmp);
		{
			currentRoom = path[0];
			path.erase(path.begin(), path.begin() + 1);
		}
	}
	else
		findPath();
	return currentRoom;

}

DCS::MobileEntity::MobileEntity(Room * current, Point location, MobileEntityType type)
{
	currentRoom = current;
	position = location;
	this->type = type;
}

DCS::Room * DCS::MobileEntity::location()
{
	return currentRoom;
}

DCS::Room * DCS::StaticEntity::update()
{
	throw std::runtime_error("not implemented");
}
