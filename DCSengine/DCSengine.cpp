#include "pch.h"
#include "DCSengine.h"
#include <cmath>
#include <ctime>


bool contains(std::vector<DCS::Room*>&t, DCS::Room*k)
{
	for (std::vector<DCS::Room*>::iterator i = t.begin(); i != t.end(); i++)
		if (*i == k)
			return true;
	return false;
}

void DCS::Game::gameTick()
{
	ship.update();
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

int DCS::MobileEntity::findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * targetRoom, DCS::Point door, DCS::Point currentPosition, std::unordered_map<Room*, std::pair<DCS::Point,int>>*prevVisits)
{
	if (location == nullptr)
		return MAXINT;
	if (location == targetRoom)
		return (int)magnitude(door - currentPosition) + currPath.second;
	std::pair<std::vector<DCS::Room*>, int> best;
	int tmpvalue;
	std::pair < DCS::Room*, DCS::Point> tmpDoor;
	std::pair<std::vector<DCS::Room*>, int> tmp;
	
	tmpDoor = location->leftDoor();
	tmp = currPath;
	std::unordered_map<Room*, std::pair<DCS::Point, int>>::iterator dooriterator = prevVisits->find(currentRoom);
	if (dooriterator == prevVisits->cend() || (dooriterator != prevVisits->cend() && (dooriterator->second.first != tmpDoor.second || magnitude(door - tmpDoor.second) + tmp.second < dooriterator->second.second)))
	{
		tmp.second += (int)magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, targetRoom, tmpDoor.second, currentPosition, prevVisits);
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
		tmp.second +=(int) magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, targetRoom, tmpDoor.second, currentPosition, prevVisits);
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
		tmp.second +=(int) magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, targetRoom, tmpDoor.second, currentPosition, prevVisits);
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
		tmp.second +=(int) magnitude(door - tmpDoor.second);
		if (dooriterator == prevVisits->cend())
			prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
		else
			dooriterator->second.second = tmp.second;
		if (!contains(tmp.first, tmpDoor.first))
		{
			tmp.first.push_back(tmpDoor.first);
			tmpvalue = findRoute(tmp, currentBest, tmpDoor.first, targetRoom, tmpDoor.second, currentPosition, prevVisits);
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
		if (pnpoly((*i)->silvete, (float)tmp.first, (float)tmp.second))
			return *i;
	}
	return nullptr;
}

void DCS::Ship::update()
{
	double oxygen = 0.0;

	for each (Room* var in rooms)
	{
		oxygen += var->generateOxygen();
	}
	for each (Room* var in rooms)
	{
		oxygen = var->supplyOxygen(oxygen);
	}

	for (std::vector<MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++)
	{
		Room* tmp = (*i)->location();
		if (tmp != (*i)->update())
		{
			tmp->removeEntity(*i);
			(*i)->location()->addEntity(*i);
		}
	}
	for (std::vector<Room*>::iterator i = rooms.begin(); i != rooms.end(); i++)
		(*i)->update();
}

DCS::Ship::Ship()
{
	srand((unsigned int)time(NULL));
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

bool DCS::Room::isOnFire()
{
	return onFire;
}

void DCS::Room::update()
{
	if (onFire)
	{
		if (oxygenLevel < MIN_OXYGEN_TO_FIRE)
		{
			onFire = false;
			fire = 0;
		}
		damage(FIRE_DAMAGE_RATE*fire);
		oxygenLevel -= fire*OXYGEN_CONSUMPTION;
		if(fire<MAX_FIRE_VALUE)
			fire +=FIRE_SPREAD_MODIFIER*fire;
		if(fire>MIN_FIRE_VALUE+ rand()%FIRE_SPREAD_CHANCE)
			switch (rand() % 4)
			{
			case 0:
				if (left != nullptr)
					left->setOnFire(log(fire/10)/10.0);
				break;
			case 1:
				if (right != nullptr)
					right->setOnFire(log(fire/10)/10.0);
				break;
			case 2:
				if (down != nullptr)
					down->setOnFire(log(fire/10)/10.0);
				break;
			case 3:
				if (up != nullptr)
					up->setOnFire(log(fire/10)/10.0);
				break;
			}
	}
}

DCS::Room::Room(Point position, std::vector<Point> Silvete, std::vector<StaticEntity> entities, RoomType type, double OxygenGeneration)
{
	this->position = position;
	this->silvete = Silvete;
	staticEntities = entities;
	this->type = type;
	oxygenGeneration = OxygenGeneration;
}

void DCS::Room::setUp(Room *room, Point connection)
{
	if (this->up == nullptr)
	{
		up = room;
		positionUp = connection;
	}
}

void DCS::Room::setDown(Room *room, Point connection)
{
	if (this->down == nullptr)
	{
		down = room;
		positionDown = connection;
	}
}

void DCS::Room::setLeft(Room *room, Point connection)
{
	if (this->left == nullptr)
	{
		left = room;
		positionLeft = connection;
	}
}

void DCS::Room::setRight(Room *room, Point connection)
{
	if (this->right == nullptr)
	{
		right = room;
		positionRight = connection;
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
			if (rand() % 100 < 30)
			{
				this->onFire = true;
				fire += FIRE_SPREAD_MODIFIER*fire;
			}
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

void DCS::Room::setOnFire()
{
	if (oxygenLevel > 1.1* MIN_OXYGEN_TO_FIRE)
	{
		fire = min(max(FIRE_START_VALUE, fire), MAX_FIRE_VALUE);
		onFire = true;

	}
}

void DCS::Room::setOnFire(double value)
{
	if (oxygenLevel > 1.1* MIN_OXYGEN_TO_FIRE)
	{
		fire = min(fire+value, MAX_FIRE_VALUE);
		onFire = true;

	}
}

int DCS::Room::fireValue()
{
	return fire;
}

void DCS::Room::extinguish(float ammount)
{
	if (fire > 0)
		fire =max(fire- ammount,0);
}

DCS::Room::RoomType DCS::Room::whatType()
{
	return type;
}

int DCS::Room::currentOxygenLevel()
{
	return oxygenLevel;
}

void DCS::Room::setDesiredOxygen(double value)
{
	desiredOxygen = max(0, min(MAX_OXYGEN, value));
}

double DCS::Room::supplyOxygen(double value)
{
	double tmp = oxygenLevel;
	oxygenLevel = max(0, min(desiredOxygen, oxygenLevel + value));
	return oxygenLevel - tmp;
}

double DCS::Room::generateOxygen()
{
	return oxygenGeneration;
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
	return (float)sqrt(op.first*op.first + op.second*op.second);
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
	throw std::exception();
}

void DCS::MobileEntity::findPath()
{
	path.clear();
	std::unordered_map<Room*, std::pair<DCS::Point, int>>map;
	std::pair<std::vector<DCS::Room*>, int>tmp(path, 0);
	findRoute(tmp, MAXINT, currentRoom, destination.second, position, destination.first, &map);
	path = tmp.first;
}

DCS::Room * DCS::MobileEntity::update()
{
	if (currentRoom->isOnFire())
		if (currentRoom->fireValue()*(rand() % 3) > DEATH_CHANCE&&health!=Dead)
			health = (Status)(health + 1);
	if (health != Nominal)
		return currentRoom;
	if (destination.second == currentRoom&&position == destination.first)
	{
		if (type == Engineer)
		{
			if (currentRoom->isOnFire())
				currentRoom->extinguish(1);
			else
				currentRoom->repair(1);
		}
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
		Point tmp;
		try
		{
			tmp = findDoor(path[0]);
		}
		catch (std::exception)
		{
			findPath();
			tmp = findDoor(path[0]);
		}
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

void DCS::MobileEntity::changeDestination(std::pair<Point, Room*> d)
{
	destination = d;
}

DCS::Room * DCS::StaticEntity::update()
{
	throw std::runtime_error("not implemented");
}

std::pair<DCS::Room*, DCS::Point> DCS::Door::otherSide(Room * curr)
{
	if (curr == firstSide.first)
		return secondSide;
	if (curr == secondSide.first)
		return firstSide;
	throw std::runtime_error("Requested room is not connected");
}

bool DCS::Door::weld(float amount)
{
	if (close())
		if (isWeldedShut < MAX_WELD)
		{
			isWeldedShut += amount;
			return true;
		}
	return false;
}

void DCS::Door::unweld(float amount)
{
	if (isWeldedShut > 0)
		isWeldedShut -= amount;
	if (isWeldedShut < 0)
		isWeldedShut = 0.0f;
}

bool DCS::Door::open()
{
	if (isOpen)
		return true;
	if (works&&isWeldedShut == 0)
	{
		isOpen = true;
		return true;
	}
	return false;
}

bool DCS::Door::close()
{
	if (this->works == true)
	{
		isOpen = false;
		return true;
	}
	else
		if (isOpen == false)
			return true;
	return false;
}

bool DCS::Door::isItOpen()
{
	return isOpen;
}

float DCS::Door::isWelded()
{
	return isWeldedShut;
}

bool DCS::Door::isOperational()
{
	return works;
}

DCS::Door::Door(Room *r1, Point p1, Room *r2, Point p2)
{
	firstSide = std::pair<Room*, Point>(r1, p1);
	secondSide = std::pair<Room*, Point>(r2, p2);
}
