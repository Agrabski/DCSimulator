#include "pch.h"
#include "DCSengine.h"
#include <cmath>


bool contains(std::vector<DCS::Room*>&t, DCS::Room*k)
{
	for (std::vector<DCS::Room*>::iterator i = t.begin(); i != t.end(); i++)
		if (*i == k)
			return true;
	return false;
}

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
	for (i = 0, j = (int)vert.size() - 1; i < (int)vert.size(); j = i++) {
		if (((vert[i].second>testy) != ((vert[j].second>testy)) &&
			(testx < (vert[j].first - vert[i].first) * (testy - vert[i].second) / (vert[j].second - vert[i].second) + vert[i].first)))
			c = !c;
	}
	return c;
}

int DCS::MobileEntity::findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * destination, DCS::Point door, DCS::Point position, std::unordered_map<Room*, std::pair<DCS::Point,int>>*prevVisits)
{
	if (location == nullptr)
		return MAXINT;
	if (location == destination)
		return (int)magnitude(door - position) + currPath.second;
	std::pair<std::vector<DCS::Room*>, int> best;
	int tmpvalue;
	std::pair < DCS::Room*, DCS::Point> tmpDoor;
	std::pair<std::vector<DCS::Room*>, int> tmp;
	
	tmpDoor = location->leftDoor();
	tmp = currPath;
	std::unordered_map<Room*, std::pair<DCS::Point, int>>::iterator dooriterator = prevVisits->find(currentRoom);
	if (dooriterator == prevVisits->cend() || (dooriterator != prevVisits->cend() && (dooriterator->second.first != tmpDoor.second || magnitude(door - tmpDoor.second) + tmp.second < dooriterator->second.second)))
	{
		tmp.second += magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, destination, tmpDoor.second, position, prevVisits);
			if (currentBest > tmpvalue)
			{
				currentBest = tmpvalue;
				best = tmp;
			}
		}
		tmp = currPath;
	}

	tmpDoor = location->rightDoor();
	tmp = currPath;
	dooriterator = prevVisits->find(currentRoom);
	if (dooriterator == prevVisits->cend() || (dooriterator != prevVisits->cend() && (dooriterator->second.first != tmpDoor.second || magnitude(door - tmpDoor.second) + tmp.second < dooriterator->second.second)))
	{
		tmp.second += magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, destination, tmpDoor.second, position, prevVisits);
			if (currentBest > tmpvalue)
			{
				currentBest = tmpvalue;
				best = tmp;
			}
		}
		tmp = currPath;
	}

	tmpDoor = location->upDoor();
	tmp = currPath;
	dooriterator = prevVisits->find(currentRoom);
	if (dooriterator == prevVisits->cend() || (dooriterator != prevVisits->cend() && (dooriterator->second.first != tmpDoor.second || magnitude(door - tmpDoor.second) + tmp.second < dooriterator->second.second)))
	{
		tmp.second += magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, destination, tmpDoor.second, position, prevVisits);
			if (currentBest > tmpvalue)
			{
				currentBest = tmpvalue;
				best = tmp;
			}
		}
		tmp = currPath;
	}
	tmp = currPath;

	tmpDoor = location->downDoor();
	tmp = currPath;
	dooriterator = prevVisits->find(currentRoom);
	if (dooriterator == prevVisits->cend() || (dooriterator != prevVisits->cend() && (dooriterator->second.first != tmpDoor.second || magnitude(door - tmpDoor.second) + tmp.second < dooriterator->second.second)))
	{
		tmp.second += magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, destination, tmpDoor.second, position, prevVisits);
			if (currentBest > tmpvalue)
			{
				currentBest = tmpvalue;
				best = tmp;
			}
		}
		tmp = currPath;
	}
	tmp = currPath;


	currPath = best;
	return currentBest;
}

DCS::MobileEntity::Status DCS::MobileEntity::currentStatus()
{
	return health;
}

DCS::Room * DCS::Ship::findRoom(Point p)
{
	for (std::vector<Room*>::iterator i = rooms.begin(); i != rooms.end(); i++)
	{
		Point tmp = p - (*i)->position;
		if (pnpoly((*i)->silvete, tmp.first, tmp.second))
			return *i;
	}
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
	corridorSilvete.emplace_back(Point(40, 0));
	corridorSilvete.emplace_back(Point(40, 20));
	corridorSilvete.emplace_back(Point(0, 20));
	Room*corridor = new Room(Point(50, 25), corridorSilvete, e, Room::Corridor);

	Room* engineering = new Room(Point(90, 10), bridgeSilvete, e, Room::RoomType::Engineering);


	MobileEntity*entity = new MobileEntity(bridge, Point(10, 10), MobileEntity::Engineer);
	MobileEntity*entity1 = new MobileEntity(bridge, Point(30, 10), MobileEntity::Marine);
	bridge->setRight(corridor, corridor->position+Point(0,10)-bridge->position);
	corridor->setLeft(bridge, DCS::Point(0, 10));
	corridor->setRight(engineering, DCS::Point(40, 10));
	engineering->setLeft(corridor,DCS::Point(0,25));
	bridge->addEntity(entity);
	bridge->addEntity(entity1);
	rooms.push_back(bridge);
	rooms.push_back(corridor);
	rooms.push_back(engineering);
	mobileEntities.push_back(entity);
	mobileEntities.push_back(entity1);
}

DCS::Ship::~Ship()
{
	for (int i = 0; i < rooms.size(); i++)
		delete rooms[i];

	for (int i = 0; i < mobileEntities.size(); i++)
		delete mobileEntities[i];
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
		positionUp = position;
	}
}

void DCS::Room::setDown(Room *room, Point position)
{
	if (this->down == nullptr)
	{
		down = room;
		positionDown = position;
	}
}

void DCS::Room::setLeft(Room *room, Point position)
{
	if (this->left == nullptr)
	{
		left = room;
		positionLeft = position;
	}
}

void DCS::Room::setRight(Room *room, Point position)
{
	if (this->right == nullptr)
	{
		right = room;
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

void DCS::Room::damage(float damage)
{
	if (state != Destroyed)
	{
		this->damageTransition -= damage;
		if (damageTransition <= 0)
		{
			state = (DamageState)(state + 1);
			damageTransition = 100.0f + damageTransition;
		}
	}
}

void DCS::Room::repair(float amount)
{
		this->damageTransition += amount;
		if (damageTransition >= 100.0f)
		{
			if(state!=Operational)
				state = (DamageState)(state - 1);
			damageTransition = 0;
		}
}

std::pair<float, DCS::DamageState> DCS::Room::currentState()
{
	return std::pair<float, DCS::DamageState>(damageTransition,state);
}

bool DCS::Room::colides(Point p, MobileEntity* e)
{
	for each (auto var in mobileEntities)
	{
		if (e != var && magnitude(var->position - p) <= 12)
			return true;
	}
	return false;
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
	std::unordered_map<Room*, std::pair<DCS::Point, int>>map;
	std::pair<std::vector<DCS::Room*>, int>tmp(path, 0);
	findRoute(tmp, MAXINT, currentRoom, destination.second, position, destination.first, &map);
	path = tmp.first;
}

DCS::Room * DCS::MobileEntity::update()
{
	if (health != Nominal)
		return currentRoom;
	if (destination.second == nullptr)
		if (type==Engineer)
		{
			currentRoom->repair(1);
			return currentRoom;
		}
	if (destination.second == currentRoom)
	{
		if (!currentRoom->colides(position + DCS::Point((destination.first.first - position.first) > 0 ? 1 : ((destination.first.first - position.first) == 0 ? 0 : -1), (destination.first.second - position.second) > 0 ? 1 : ((destination.first.second - position.second) == 0 ? 0 : -1)), this))
			position = position + DCS::Point((destination.first.first - position.first) > 0 ? 1 : ((destination.first.first - position.first) == 0 ? 0 : -1), (destination.first.second - position.second) > 0 ? 1 : ((destination.first.second - position.second) == 0 ? 0 : -1));
		else
			if (!currentRoom->colides(destination.first, this))
				if (!currentRoom->colides(position + DCS::Point(2, 0), this) && pnpoly(currentRoom->silvete, (position + DCS::Point(2, 0)).first, (position + DCS::Point(2, 0)).second))
					position = position + DCS::Point(2, 0);
				else
					if (!currentRoom->colides(position + DCS::Point(0, 2), this) && pnpoly(currentRoom->silvete, (position + DCS::Point(0, 2)).first, (position + DCS::Point(0, 2)).second))
						position = position + DCS::Point(0, 2);
					else
						if (!currentRoom->colides(position + DCS::Point(-2, 0), this) && pnpoly(currentRoom->silvete, (position + DCS::Point(-2, 0)).first, (position + DCS::Point(-2, 0)).second))
							position = position + DCS::Point(-2, 0);
						else
							if (!currentRoom->colides(position + DCS::Point(0, -2), this) && pnpoly(currentRoom->silvete, (position + DCS::Point(0, -2)).first, (position + DCS::Point(0, -2)).second))
								position = position + DCS::Point(0, -2);


	}
	if (path.size() != 0 && (destination.second == path[path.size() - 1]))
	{
		Point tmp = findDoor(path[0]);
		Room*prev=currentRoom;
		position = position + DCS::Point((tmp.first - position.first) > 0 ? 1 : ((tmp.first - position.first) == 0 ? 0 : -1), (tmp.second - position.second) > 0 ? 1 : ((tmp.second - position.second) == 0 ? 0 : -1));
		if (position == tmp)
		{
			currentRoom = path[0];
			position = findDoor(prev);
			if (!currentRoom->colides(position, this))
				path.erase(path.begin(), path.begin() + 1);
			else
			{
				currentRoom = prev;
				position = tmp;
			}

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
