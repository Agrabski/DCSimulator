#pragma once
#include <vector>
#include <stdio.h>
#include <unordered_map>
#include <typeinfo>
#define MAX_FIRE_VALUE 1000
#pragma region Defs

#define REPAIR_SPEED 1.0f
#define FIRE_DAMAGE_RATE 0.0005f
#define FIRE_SPREAD_MODIFIER .001f
#define FIRE_SPREAD_CHANCE 200
#define MIN_FIRE_VALUE 500
#define DEATH_CHANCE 95*MAX_FIRE_VALUE
#define MAX_OXYGEN 100.0f
#define MAX_OVERPRESSURE 150.0
#define MIN_OXYGEN_TO_FIRE 20.0f
#define MAX_OXYGEN_SUPPLY_RATE 2.0f
#define MAX_WELD 100.0f
#define OXYGEN_CONSUMPTION 0.00005f
#define FIRE_START_VALUE 10
#define DAMAGE_EFFICENCY_MULTIPLIER .5
#define EXTINGUISH_CONSTANT .5
#define MAX_OXYGEN_FLOW 1.0

#pragma endregion


namespace DCS
{
	typedef std::pair<int, int> Point;

	Point operator+(const Point&left, const Point&right);
	Point operator-(const Point&left, const Point&right);
	bool operator<(const Point&left, const Point&right);
	double magnitude(const Point&op);

	class Room;
	class Entity;
	class MobileEntity;
	class StaticEntity;
	class Objective;
	class Scenario;
	class Event;
	class HullBreach;
	class Door;


	enum ScenarioResult { Continue, Lost, Won };
	enum DamageState { Operational, Damaged, OutOfAction, Destroyed };

	class Ship
	{
		std::vector<HullBreach*>breaches;
		std::vector<Room*>rooms;
		std::vector<Door*>doors;
	public:
		std::vector<HullBreach*>::iterator breachBegin();
		std::vector<HullBreach*>::iterator breachEnd();
		Room*breachesTop();
		std::vector<Point>silvete;
		void addBreach(HullBreach*);
		std::vector<MobileEntity*>mobileEntities;
		Room *findRoom(Point);
		std::vector<Room*>::iterator roomBegin();
		std::vector<Room*>::const_iterator roomCbegin();
		std::vector<Room*>::const_iterator roomCend();
		std::vector<MobileEntity*>::const_iterator entityCbegin();
		std::vector<MobileEntity*>::const_iterator entityCend();
		std::vector<Door*>::iterator doorBegin();
		std::vector<Door*>::iterator doorEnd();
		void update();
		Ship();
		~Ship();
	};

	class Door
	{
		std::pair<Room*, Point> firstSide;
		std::pair<Room*, Point> secondSide;
		bool isOpen;
		double isWeldedShut = 0.0f;
		bool works;
		bool stayClosed = true;
		int ticsToClose = 50;
		int ticsUntilClose = ticsToClose;
	public:
		void tick();
		std::pair<Room*, Point>otherSide(Room*curr) const;
		std::pair<Room*, Room*>rooms();
		bool weld(double amount);
		void unweld(double amount);
		//returns success state
		bool open();
		//returns success state
		bool close();
		bool isItOpen();
		double isWelded();
		bool isOperational();
		Door(Room*, Point, Room*, Point);
		Point position(Room*t);
		double leakOxygen(double value, Room*source,double precentage);
	};

	class Room
	{
		bool PlayerHasVision;
		double oxygenLevel = MAX_OXYGEN;
		std::vector<StaticEntity>staticEntities;
		std::vector<MobileEntity*>mobileEntities;
		bool onFire = false;
		double fire = 0;
		int volume=1;
		std::vector<Door*> doors;
		DamageState state;
		double damageTransition = 100.0f;
		double oxygenGeneration = 0;
		double desiredOxygen = MAX_OXYGEN;
	public:
		std::vector<Door*>::iterator giveDoors();
		std::vector<Door*>::iterator doorsEnd();
		Door& findDoor(Room*next);
		bool isOnFire() const;
		Point position;
		std::vector<Point>silvete;
		enum RoomType { Bridge, Engineering, Corridor, LifeSupport };
		void update();
		Room(Point position, std::vector<Point>Silvete, std::vector<StaticEntity>entities, RoomType type, double OxygenGeneration = 0);
		void setConnection(Door*);
		void removeEntity(MobileEntity*e);
		void addEntity(MobileEntity*e);
		void damage(double damage);
		void repair(double amount);
		std::pair<double, DamageState> currentState() const;
		bool colides(Point p, MobileEntity* e) const;
		void setOnFire();
		void setOnFire(double value);
		int fireValue() const;
		void extinguish(double ammount);
		RoomType whatType() const;
		int currentOxygenLevel()const;
		void setDesiredOxygen(double);
		double supplyOxygen(double value);
		double generateOxygen();
		MobileEntity*findEntity(Point);
		MobileEntity*findEntity(Point, MobileEntity*);
		void suckOxygen(double value);
		void forceOxygen(double value);
		double roomVolume();
	private:
		RoomType type;

	};

	class Entity
	{
	public:
		//returns pointer to the current room
		virtual Room* update() = 0;
		Point position;
		Room*currentRoom;
	protected:


	};

	class MobileEntity :public Entity
	{
	public:
		enum Status { Nominal, Wounded, Dead };
	private:
		std::pair<Point, bool>tmpDestination = std::pair<Point, bool>(Point(0, 0), false);
		Status health = Nominal;
		std::vector<Room*>path;
		void findPath();
		int findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * destination, DCS::Point door, DCS::Point position, std::unordered_map<Room*, std::pair<DCS::Point, int>>*);
	public:
		std::vector<Room*> currentPath();
		Status currentStatus();
		std::vector<DCS::Room*>::iterator pathIterator();
		std::vector<DCS::Room*>::iterator end();
		bool selected = false;
		//returns pointer to the current room
		virtual Room* update();
		enum MobileEntityType { Marine, Engineer, CrewMember, Boarder }type;
		MobileEntity(Room*current, Point location, MobileEntityType type);
		std::pair<Point, Room*> destination;
		Room*location();
		void changeDestination(std::pair<Point, Room*> d);
		void move(Point contestedPoint);
	};

	class StaticEntity :public Entity
	{
	public:
		//returns pointer to the current room
		virtual Room* update();
		enum StaticEntityType {}type;
	};

	class Game
	{
	protected:
		enum ScreenType { MainMenu, InGame } CurrentScreen = MainMenu;
		bool isPaused = false;
		ScenarioResult state = Continue;
	public:
		Game();
		std::vector<MobileEntity*>selected;
		Scenario* currentScenario;
		void gameTick();
		void switchPause();
	};


	class Scenario
	{
		int gameTimer = 0;
		std::vector<Event*> events;
	public:
		Ship* ship;
		std::vector<Objective*> target;
		std::ofstream& operator<<(std::ofstream&);
		std::ifstream& operator>>(std::ifstream&);
		Scenario(std::vector<Objective*>objective, Ship * vessel);
		Scenario(std::vector<Objective*>objective, Ship * vessel, std::vector<Event*> &events);
		Scenario(std::ifstream & file);
		ScenarioResult scenarioTick();
		void addObjective(Objective*obj);
		std::vector<int> ticsRemaining();
		std::vector <std::pair <Room*, DamageState>> roomsRequired();
		~Scenario();
	};

	class Objective
	{
	protected:
		std::vector <std::pair <Room*, DamageState>>requieredRooms;
		const Ship* gameReference;
	public:
		Objective(Ship * ref);
		virtual bool isFullfilled(int ticCount) = 0;
		virtual bool isFailed(int ticCount) = 0;
		std::vector <std::pair <Room*, DamageState>> requiredRooms();
	};

	class Timed : public  Objective
	{
		int ticsRemaining;
	public:
		Timed(std::vector <std::pair <Room*, DamageState>>reqRooms, Ship*ref, int tics);
		virtual bool isFullfilled(int ticCount);
		virtual bool isFailed(int ticCount);
		int timeRemaining(int tics);
	};

	class Trigger
	{
	protected:
		bool triggered = false;
	public:
		virtual bool operator()(int tics, Scenario*ref) = 0;
	};

	class TimeTrigger :public Trigger
	{
		int ticsToTrigger;
	public:
		virtual bool operator()(int tics, Scenario*ref);
		TimeTrigger(int n);
	};

	class Event
	{
		std::vector<std::pair<Room*, int>>setOnFire;
		std::vector<std::pair<Room*, int>>toBoard;
		std::vector<Room*>toDepressurise;
		std::vector<std::pair<Room*, int>>toDamage;
		Trigger*trigger;
	public:
		void tryTrigger(int tics, Scenario* ref);
		Event(Trigger* t, std::vector<std::pair<Room*, int>>setOnFire, std::vector<std::pair<Room*, int>>toBoard, std::vector<Room*>toDepressurise, std::vector<std::pair<Room*, int>>toDamage);
		Event& operator=(Event&);
		Event(const Event&t) = default;
		~Event();
	};

	class HullBreach
	{
		Room*affectedRoom;
		double size;
		const static double pressureOverSizeToEscalate;
		const static int maxSize;
		static double oxygenFlowMultiplier;
		static int pullMultiplier;
		Point absolutePosition;
	public:
		HullBreach(Room*, double size, Point p);
		void timerTick();
		Room *affected() const;
		Point position() const;
		int whatSize() const;
	};
}