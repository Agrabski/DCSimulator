#include "pch.h"
#include "DCSengine.h"
#include <cmath>
#include <ctime>


const double DCS::HullBreach::pressureOverSizeToEscalate = .75;
const int DCS::HullBreach::maxSize = 100;
double DCS::HullBreach::oxygenFlowMultiplier = .01;


bool contains(std::vector<DCS::Room*>&t, DCS::Room*k)
{
	for ( std::vector<DCS::Room*>::iterator i = t.begin(); i != t.end(); i++ )
		if ( *i == k )
			return true;
	return false;
}

DCS::Game::Game()
{
}

void DCS::Game::gameTick()
{
	if ( !isPaused&&currentScenario != nullptr&&state == Continue&&CurrentScreen == InGame )
		state = currentScenario->scenarioTick();
}

void DCS::Game::switchPause()
{
	isPaused = !isPaused;
}


bool pnpoly(std::vector<DCS::Point>vert, double testx, double testy)
{
	int i, j;
	bool c = false;
	for ( i = 0, j = (int)vert.size() - 1; i < (int)vert.size(); j = i++ ) {
		if ( ( ( vert[i].second > testy ) != ( ( vert[j].second > testy ) ) &&
			( testx < ( vert[j].first - vert[i].first ) * ( testy - vert[i].second ) / ( vert[j].second - vert[i].second ) + vert[i].first ) ) )
			c = !c;
	}
	return c;
}

int DCS::MobileEntity::findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * targetRoom, DCS::Point door, DCS::Point currentPosition, std::unordered_map<Room*, std::pair<DCS::Point, int>>*prevVisits)
{
	if ( location == nullptr )
		return MAXINT;
	if ( location == targetRoom )
		return (int)magnitude(door - currentPosition) + currPath.second;
	std::pair<std::vector<DCS::Room*>, int> best;
	int tmpvalue;
	std::pair<std::vector<DCS::Room*>, int> tmp;

	for ( std::vector<Door*>::iterator tmpDoor = location->giveDoors(); tmpDoor != location->doorsEnd(); tmpDoor++ )
	{
		tmp = currPath;
		std::unordered_map<Room*, std::pair<DCS::Point, int>>::iterator dooriterator = prevVisits->find(currentRoom);
		if ( dooriterator == prevVisits->cend() || ( dooriterator != prevVisits->cend() && ( dooriterator->second.first != ( *tmpDoor )->position(location) || magnitude(door - ( *tmpDoor )->position(location)) + tmp.second < dooriterator->second.second ) ) )
		{
			tmp.second += (int)magnitude(door - ( *tmpDoor )->position(location));
			if ( dooriterator == prevVisits->cend() )
				prevVisits->insert(std::pair<DCS::Room*, std::pair<DCS::Point, int>>(currentRoom, std::pair<DCS::Point, int>(door, tmp.second)));
			else
				dooriterator->second.second = tmp.second;
			if ( !contains(tmp.first, ( *tmpDoor )->otherSide(location).first) )
			{
				tmp.first.push_back(( *tmpDoor )->otherSide(location).first);
				tmpvalue = findRoute(tmp, currentBest, ( *tmpDoor )->otherSide(location).first, targetRoom, ( *tmpDoor )->position(location), currentPosition, prevVisits);
				if ( currentBest > tmpvalue )
				{
					currentBest = tmpvalue;
					best = tmp;
				}
			}
			tmp = currPath;
		}
	}

	currPath = best;
	return currentBest;
}

std::vector<DCS::Room*> DCS::MobileEntity::currentPath()
{
	return path;
}

DCS::MobileEntity::Status DCS::MobileEntity::currentStatus()
{
	return health;
}

std::vector<DCS::Room*>::iterator DCS::MobileEntity::pathIterator()
{
	return path.begin();
}

std::vector<DCS::Room*>::iterator DCS::MobileEntity::end()
{
	return path.end();
}

void DCS::Ship::addBreach(HullBreach *arg)
{
	breaches.push_back(arg);
}

DCS::Room * DCS::Ship::findRoom(Point p)
{
	for ( std::vector<Room*>::iterator i = rooms.begin(); i != rooms.end(); i++ )
	{
		Point tmp = p - ( *i )->position;
		if ( pnpoly(( *i )->silvete, (double)tmp.first, (double)tmp.second) )
			return *i;
	}
	return nullptr;
}

std::vector<DCS::Room*>::iterator DCS::Ship::roomBegin()
{
	return rooms.begin();
}

std::vector<DCS::Room*>::const_iterator DCS::Ship::roomCbegin()
{
	return rooms.cbegin();
}

std::vector<DCS::Room*>::const_iterator DCS::Ship::roomCend()
{
	return rooms.cend();
}

std::vector<DCS::MobileEntity*>::const_iterator DCS::Ship::entityCbegin()
{
	return mobileEntities.cbegin();
}

std::vector<DCS::MobileEntity*>::const_iterator DCS::Ship::entityCend()
{
	return mobileEntities.cend();
}

std::vector<DCS::Door*>::iterator DCS::Ship::doorBegin()
{
	return doors.begin();
}

std::vector<DCS::Door*>::iterator DCS::Ship::doorEnd()
{
	return doors.end();
}

void DCS::Ship::update()
{
	double oxygen = 0.0;

	for each ( Room* var in rooms )
	{
		oxygen += var->generateOxygen();
	}
	for each ( Room* var in rooms )
	{
		oxygen = var->supplyOxygen(oxygen);
	}
	for each ( auto var in breaches )
	{
		var->timerTick();
	}

	for each ( auto var in doors )
	{
		var->tick();
	}

	for ( std::vector<MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++ )
	{
		Room* tmp = ( *i )->location();
		if ( tmp != ( *i )->update() )
		{
			tmp->removeEntity(*i);
			( *i )->location()->addEntity(*i);
		}
	}
	for ( std::vector<Room*>::iterator i = rooms.begin(); i != rooms.end(); i++ )
		( *i )->update();
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

	Room* lifeSupport = new Room(Point(90, 95), bridgeSilvete, e, Room::RoomType::LifeSupport, .03);


	MobileEntity*entity = new MobileEntity(bridge, Point(10, 10), MobileEntity::Marine);
	MobileEntity*entity1 = new MobileEntity(lifeSupport, Point(30, 10), MobileEntity::Engineer);
	MobileEntity*entity2 = new MobileEntity(lifeSupport, Point(10, 10), MobileEntity::Engineer);
	Door*tmp = new Door(bridge, DCS::Point(40, 25), corridor, DCS::Point(0, 10));
	bridge->setConnection(tmp);
	corridor->setConnection(tmp);
	doors.push_back(tmp);
	tmp = new Door(corridor, DCS::Point(40, 10), engineering, DCS::Point(0, 25));
	corridor->setConnection(tmp);
	engineering->setConnection(tmp);
	doors.push_back(tmp);
	tmp = new Door(engineering, Point(20, 85), lifeSupport, Point(20, 0));
	engineering->setConnection(tmp);
	lifeSupport->setConnection(tmp);
	doors.push_back(tmp);
	bridge->addEntity(entity);
	lifeSupport->addEntity(entity1);
	rooms.push_back(bridge);
	rooms.push_back(corridor);
	rooms.push_back(engineering);
	rooms.push_back(lifeSupport);
	mobileEntities.push_back(entity);
	mobileEntities.push_back(entity1);
	mobileEntities.push_back(entity2);
}

DCS::Ship::~Ship()
{
	for ( int i = 0; i < rooms.size(); i++ )
		delete rooms[i];

	for ( int i = 0; i < mobileEntities.size(); i++ )
		delete mobileEntities[i];
}

std::vector<DCS::Door*>::iterator DCS::Room::giveDoors()
{
	return doors.begin();
}

std::vector<DCS::Door*>::iterator DCS::Room::doorsEnd()
{
	return doors.end();
}

DCS::Door& DCS::Room::findDoor(Room * next)
{
	std::pair<Room*, Point> k;
	for ( int i = 0; i < doors.size(); i++ )
		if ( doors[i]->otherSide(this).first == next )
			return *doors[i];
	throw std::exception();
}

bool DCS::Room::isOnFire() const
{
	return onFire;
}

void DCS::Room::update()
{
	if ( onFire )
	{
		if ( oxygenLevel < MIN_OXYGEN_TO_FIRE )
		{
			onFire = false;
			fire = 0;
		}
		damage(FIRE_DAMAGE_RATE*fire);
		oxygenLevel -= fire*OXYGEN_CONSUMPTION;
		if ( fire < MAX_FIRE_VALUE )
			fire += FIRE_SPREAD_MODIFIER*fire;
		if ( fire > MIN_FIRE_VALUE + rand() % FIRE_SPREAD_CHANCE )
			doors[rand() % doors.size()]->otherSide(this).first->setOnFire(log(fire / 10) / 10.0);
	}
	for each( auto t in doors )
		oxygenLevel -= t->leakOxygen(min(oxygenLevel*volume, MAX_OXYGEN_FLOW), this, oxygenLevel) / volume;

}

DCS::Room::Room(Point position, std::vector<Point> Silvete, std::vector<StaticEntity> entities, RoomType type, double OxygenGeneration)
{
	//TODO: calculate volume
	this->position = position;
	this->silvete = Silvete;
	staticEntities = entities;
	this->type = type;
	oxygenGeneration = OxygenGeneration;
}

void DCS::Room::setConnection(Door *t)
{
	doors.push_back(t);
}


void DCS::Room::removeEntity(MobileEntity * e)
{
	for ( std::vector<MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++ )
	{
		if ( *i == e )
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

void DCS::Room::damage(double damage)
{
	if ( state != Destroyed )
	{
		this->damageTransition -= damage;
		if ( damageTransition <= 0 )
		{
			state = (DamageState)( state + 1 );
			damageTransition = 100.0f + damageTransition;
			if ( rand() % 100 < 30 )
			{
				this->onFire = true;
				fire += FIRE_SPREAD_MODIFIER*fire;
			}
		}
	}
}

void DCS::Room::repair(double amount)
{
	this->damageTransition += amount;
	if ( damageTransition >= 100.0f )
	{
		if ( state != Operational )
			state = (DamageState)( state - 1 );
		damageTransition = 0;
	}
}

std::pair<double, DCS::DamageState> DCS::Room::currentState() const
{
	return std::pair<double, DCS::DamageState>(damageTransition, state);
}

bool DCS::Room::colides(Point p, MobileEntity* e) const
{
	for each ( auto var in mobileEntities )
	{
		if ( e != var && magnitude(var->position - p) <= 12 )
			return true;
	}
	return false;
}

void DCS::Room::setOnFire()
{
	if ( oxygenLevel > 1.1* MIN_OXYGEN_TO_FIRE )
	{
		fire = min(max(FIRE_START_VALUE, fire), MAX_FIRE_VALUE);
		onFire = true;

	}
}

void DCS::Room::setOnFire(double value)
{
	if ( oxygenLevel > 1.1* MIN_OXYGEN_TO_FIRE )
	{
		fire = min(fire + value, MAX_FIRE_VALUE);
		onFire = true;

	}
}

int DCS::Room::fireValue() const
{
	return (int)fire;
}

void DCS::Room::extinguish(double ammount)
{
	if ( fire > 0 )
		fire = max(fire - ammount, 0);
	if ( fire == 0 )
		onFire = false;
}

DCS::Room::RoomType DCS::Room::whatType() const
{
	return type;
}

int DCS::Room::currentOxygenLevel() const
{
	return (int)oxygenLevel;
}

void DCS::Room::setDesiredOxygen(double value)
{
	desiredOxygen = max(0, min(MAX_OXYGEN, value));
}

double DCS::Room::supplyOxygen(double value)
{
	double tmp = oxygenLevel;
	oxygenLevel =max(0, max(oxygenLevel-.1, min(desiredOxygen, oxygenLevel + value)));
	return oxygenLevel - tmp != 0 ? oxygenLevel - tmp<0?value- oxygenLevel + tmp : oxygenLevel - tmp : value;
}

double DCS::Room::generateOxygen()
{
	if ( state == Operational )
		return oxygenGeneration;
	else
		if ( state == Damaged )
			return oxygenGeneration * DAMAGE_EFFICENCY_MULTIPLIER;
		else
			return 0;
}

DCS::MobileEntity * DCS::Room::findEntity(Point p)
{
	for ( std::vector<DCS::MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++ )
		if ( magnitude(( *i )->position - p) < 20 )
		{
			return *i;
		}
}

DCS::MobileEntity * DCS::Room::findEntity(Point p, MobileEntity *t)
{
	for ( std::vector<DCS::MobileEntity*>::iterator i = mobileEntities.begin(); i != mobileEntities.end(); i++ )
		if ( magnitude(( *i )->position - p) < 20 && t != *i )
		{
			return *i;
		}
}

void DCS::Room::suckOxygen(double value)
{
	oxygenLevel = max(0, oxygenLevel - value);
}

void DCS::Room::forceOxygen(double value)
{
	oxygenLevel =min(oxygenLevel+ value / volume,MAX_OVERPRESSURE);
}

double DCS::Room::roomVolume()
{
	return volume;
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

double DCS::magnitude(const Point & op)
{
	return (double)sqrt(op.first*op.first + op.second*op.second);
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
	if ( currentRoom->isOnFire() )
		if ( currentRoom->fireValue()*( rand() % 100 ) > DEATH_CHANCE&&health != Dead )
			health = (Status)( health + 1 );
	if ( health != Nominal )
		return currentRoom;

	if ( tmpDestination.second )
	{
		if ( !currentRoom->colides(position + DCS::Point(( tmpDestination.first.first - position.first ) > 0 ? 1 : ( ( tmpDestination.first.first - position.first ) == 0 ? 0 : -1 ), ( tmpDestination.first.second - position.second ) > 0 ? 1 : ( ( tmpDestination.first.second - position.second ) == 0 ? 0 : -1 )), this) )
			position = position + DCS::Point(( tmpDestination.first.first - position.first ) > 0 ? 1 : ( ( tmpDestination.first.first - position.first ) == 0 ? 0 : -1 ), ( tmpDestination.first.second - position.second ) > 0 ? 1 : ( ( tmpDestination.first.second - position.second ) == 0 ? 0 : -1 ));
		else
			if ( !currentRoom->colides(tmpDestination.first, this) )
				if ( !currentRoom->colides(position + DCS::Point(2, 0), this) && pnpoly(currentRoom->silvete, ( position + DCS::Point(2, 0) ).first, ( position + DCS::Point(2, 0) ).second) )
					position = position + DCS::Point(2, 0);
				else
					if ( !currentRoom->colides(position + DCS::Point(0, 2), this) && pnpoly(currentRoom->silvete, ( position + DCS::Point(0, 2) ).first, ( position + DCS::Point(0, 2) ).second) )
						position = position + DCS::Point(0, 2);
					else
						if ( !currentRoom->colides(position + DCS::Point(-2, 0), this) && pnpoly(currentRoom->silvete, ( position + DCS::Point(-2, 0) ).first, ( position + DCS::Point(-2, 0) ).second) )
							position = position + DCS::Point(-2, 0);
						else
							if ( !currentRoom->colides(position + DCS::Point(0, -2), this) && pnpoly(currentRoom->silvete, ( position + DCS::Point(0, -2) ).first, ( position + DCS::Point(0, -2) ).second) )
								position = position + DCS::Point(0, -2);
		if ( position == tmpDestination.first )
			tmpDestination.second = false;

		return currentRoom;
	}

	if ( destination.second == currentRoom&&position == destination.first )
	{
		if ( type == Engineer )
		{
			if ( currentRoom->isOnFire() )
				currentRoom->extinguish(EXTINGUISH_CONSTANT);
			else
				currentRoom->repair(1);
		}
		return currentRoom;

	}
	if ( destination.second == currentRoom )
	{
		if ( !currentRoom->colides(position + DCS::Point(( destination.first.first - position.first ) > 0 ? 1 : ( ( destination.first.first - position.first ) == 0 ? 0 : -1 ), ( destination.first.second - position.second ) > 0 ? 1 : ( ( destination.first.second - position.second ) == 0 ? 0 : -1 )), this) )
			position = position + DCS::Point(( destination.first.first - position.first ) > 0 ? 1 : ( ( destination.first.first - position.first ) == 0 ? 0 : -1 ), ( destination.first.second - position.second ) > 0 ? 1 : ( ( destination.first.second - position.second ) == 0 ? 0 : -1 ));
		else
			if ( magnitude(destination.first - position) < 20 )
				destination.first = position;
			else
				currentRoom->findEntity(position + DCS::Point(( destination.first.first - position.first ) > 0 ? 1 : ( ( destination.first.first - position.first ) == 0 ? 0 : -1 ), ( destination.first.second - position.second ) > 0 ? 1 : ( ( destination.first.second - position.second ) == 0 ? 0 : -1 )), this)
				->move(position + DCS::Point(( destination.first.first - position.first ) > 0 ? 1 : ( ( destination.first.first - position.first ) == 0 ? 0 : -1 ), ( destination.first.second - position.second ) > 0 ? 1 : ( ( destination.first.second - position.second ) == 0 ? 0 : -1 )));
	}
	if ( path.size() != 0 && ( destination.second == path[path.size() - 1] ) )
	{
		Point tmp;
		try
		{
			tmp = currentRoom->findDoor(path[0]).otherSide(path[0]).second;
		}
		catch ( std::exception )
		{
			findPath();
			tmp = currentRoom->findDoor(path[0]).otherSide(path[0]).second;
		}
		Room*prev = currentRoom;
		if ( !currentRoom->colides(position + DCS::Point(( tmp.first - position.first ) > 0 ? 1 : ( ( tmp.first - position.first ) == 0 ? 0 : -1 ), ( tmp.second - position.second ) > 0 ? 1 : ( ( tmp.second - position.second ) == 0 ? 0 : -1 )), this) )
			position = position + DCS::Point(( tmp.first - position.first ) > 0 ? 1 : ( ( tmp.first - position.first ) == 0 ? 0 : -1 ), ( tmp.second - position.second ) > 0 ? 1 : ( ( tmp.second - position.second ) == 0 ? 0 : -1 ));
		else
			currentRoom->findEntity(position + DCS::Point(( tmp.first - position.first ) > 0 ? 1 : ( ( tmp.first - position.first ) == 0 ? 0 : -1 ), ( tmp.second - position.second ) > 0 ? 1 : ( ( tmp.second - position.second ) == 0 ? 0 : -1 )), this)
			->move(position + DCS::Point(position + DCS::Point(( tmp.first - position.first ) > 0 ? 1 : ( ( tmp.first - position.first ) == 0 ? 0 : -1 ), ( tmp.second - position.second ) > 0 ? 1 : ( ( tmp.second - position.second ) == 0 ? 0 : -1 ))));

		if ( position == tmp )
		{
			Door& tmpDoor = currentRoom->findDoor(path[0]);
			tmpDoor.open();
			position = tmpDoor.otherSide(currentRoom).second;
			currentRoom = path[0];
			if ( !currentRoom->colides(position, this) )
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
	destination = std::pair<Point, Room*>(location, current);
}

DCS::Room * DCS::MobileEntity::location()
{
	return currentRoom;
}

void DCS::MobileEntity::changeDestination(std::pair<Point, Room*> d)
{
	destination = d;
}

void DCS::MobileEntity::move(Point contestedPoint)
{
	for ( int distance = 10; distance < 200; distance++ )
		for ( int x = -1; x <= 1; x++ )
			for ( int y = -1; y <= 1; y++ )
				if ( magnitude(position + DCS::Point(x*distance, y*distance) - contestedPoint) > 15 && pnpoly(currentRoom->silvete, ( position + DCS::Point(x*distance, y*distance) ).first, ( position + DCS::Point(x*distance, y*distance) ).second) )
				{
					tmpDestination = std::pair<DCS::Point, bool>(position + DCS::Point(x*distance, y*distance), true);
					return;
				}
}

DCS::Room * DCS::StaticEntity::update()
{
	throw std::runtime_error("not implemented");
}

void DCS::Door::tick()
{
	if ( isOpen )
	{
		if ( ticsUntilClose == 0 )
			close();
		else
			ticsUntilClose--;
	}
}

std::pair<DCS::Room*, DCS::Point> DCS::Door::otherSide(Room * curr) const
{
	if ( curr == firstSide.first )
		return secondSide;
	if ( curr == secondSide.first )
		return firstSide;
	throw std::runtime_error("Requested room is not connected");
}

std::pair<DCS::Room*, DCS::Room*> DCS::Door::rooms()
{
	return std::pair<Room*, Room*>(firstSide.first,secondSide.first);
}

bool DCS::Door::weld(double amount)
{
	if ( close() )
		if ( isWeldedShut < MAX_WELD )
		{
			isWeldedShut += amount;
			return true;
		}
	return false;
}

void DCS::Door::unweld(double amount)
{
	if ( isWeldedShut > 0 )
		isWeldedShut -= amount;
	if ( isWeldedShut < 0 )
		isWeldedShut = 0.0f;
}

bool DCS::Door::open()
{
	if ( isOpen )
		return true;
	if ( works&&isWeldedShut == 0 )
	{
		ticsUntilClose = ticsToClose;
		isOpen = true;
		return true;
	}
	return false;
}

bool DCS::Door::close()
{
	if ( this->works == true )
	{
		isOpen = false;
		return true;
	}
	else
		if ( isOpen == false )
			return true;
	return false;
}

bool DCS::Door::isItOpen()
{
	return isOpen;
}

double DCS::Door::isWelded()
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
	works = true;
	isOpen = false;
}

DCS::Point DCS::Door::position(Room * t)
{
	return t == firstSide.first ? firstSide.second : secondSide.second;
}

double DCS::Door::leakOxygen(double value, Room * source, double precentage)
{
	if(!isOpen )
		return 0.0;
	Room*tmp = otherSide(source).first;
	double n = value*max(1.0 - tmp->currentOxygenLevel() / precentage, 0.0);;

	tmp->forceOxygen(n);
	return n;
}

DCS::Objective::Objective(Ship * ref)
{
	gameReference = ref;
}

std::vector <std::pair <DCS::Room*, DCS::DamageState>> DCS::Objective::requiredRooms()
{
	return requieredRooms;
}


DCS::Scenario::Scenario(std::vector<Objective *> objective, Ship * vessel)
{
	ship = vessel;
	for each ( auto var in objective )
	{
		target.push_back(var);
	}
}

DCS::Scenario::Scenario(std::vector<Objective*> objective, Ship * vessel, std::vector<Event*> &events)
{
	ship = vessel;
	for each ( auto var in objective )
	{
		target.push_back(var);
	}
	this->events = events;
}

DCS::Scenario::Scenario(std::ifstream & file)
{
}

DCS::ScenarioResult DCS::Scenario::scenarioTick()
{
	gameTimer++;
	for each(auto var in events)
	{
		var->tryTrigger(gameTimer, this);
	}
	ship->update();
	for each ( auto var in target)
	{
	if ( var->isFullfilled(gameTimer) )
		return Won;
	if ( var->isFailed(gameTimer) )
		return Lost;

	}
	return Continue;

}

void DCS::Scenario::addObjective(Objective * obj)
{
	target.push_back(obj);
}

std::vector<int> DCS::Scenario::ticsRemaining()
{
	std::vector<int> tmp;
	for each ( auto var in target )
		if ( typeid( *var ) == typeid( Timed ) )
			tmp.push_back( ( (Timed*)var )->timeRemaining(gameTimer));
	return tmp;
}

std::vector <std::pair <DCS::Room*, DCS::DamageState>> DCS::Scenario::roomsRequired()
{
	std::vector <std::pair <Room*, DamageState>> tmp;
	for each ( auto var in target )
	{
		for each ( auto iterator in var->requiredRooms() )
		{
			tmp.push_back(iterator);
		}
	}
	return tmp;
}

DCS::Scenario::~Scenario()
{
	delete ship;
}

DCS::Timed::Timed(std::vector<std::pair<DCS::Room*, DCS::DamageState>> reqRooms, Ship *ref, int tics) : Objective(ref)
{
	ticsRemaining = tics;
	requieredRooms = reqRooms;
}

bool DCS::Timed::isFullfilled(int ticCount)
{
	return ticsRemaining < ticCount;
}

bool DCS::Timed::isFailed(int ticCount)
{
	for ( auto i = requieredRooms.begin(); i != requieredRooms.end(); i++ )
		if ( i->first->currentState().second > i->second )
			return true;
	return false;
}

int DCS::Timed::timeRemaining(int tics)
{
	return ticsRemaining - tics;
}

void DCS::Event::tryTrigger(int tics, Scenario * ref)
{
	if ( (*trigger)(tics, ref) )
	{
		for each ( auto var in setOnFire )
		{
			var.first->setOnFire((double)var.second);
			
		}
	}
}

DCS::Event::Event(Trigger * t, std::vector<std::pair<Room*, int>> setOnFire, std::vector<std::pair<Room*, int>> toBoard, std::vector<Room*> toDepressurise, std::vector<std::pair<Room*, int>> toDamage)
{
	this->setOnFire = setOnFire;
	this->toBoard = toBoard;
	this->toDepressurise = toDepressurise;
	this->toDamage = toDamage;
	trigger = t;
}

DCS::Event & DCS::Event::operator=(DCS::Event &t)
{
	this->setOnFire = t.setOnFire;
	this->toBoard = t.toBoard;
	this->toDepressurise = t.toDepressurise;
	this->toDamage = t.toDamage;
	trigger = t.trigger;
	t.trigger = nullptr;
	return *this;
}



DCS::Event::~Event()
{
	delete trigger;
}

bool DCS::TimeTrigger::operator()(int tics, Scenario * ref)
{
	if ( !triggered&& tics >= ticsToTrigger )
	{
		triggered = true;
		return true;
	}
	return false;

}

DCS::TimeTrigger::TimeTrigger(int n)
{
	ticsToTrigger = n;
}

DCS::HullBreach::HullBreach(Room *r, double size)
{
	affectedRoom = r;
	this->size = size;
}

void DCS::HullBreach::timerTick()
{
	if ( affectedRoom->currentOxygenLevel() / size > pressureOverSizeToEscalate )
		size = min(maxSize, size + affectedRoom->currentOxygenLevel() / MAX_OXYGEN * 2);
	affectedRoom->suckOxygen(size*oxygenFlowMultiplier);
}
