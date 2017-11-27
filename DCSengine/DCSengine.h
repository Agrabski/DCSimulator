#pragma once
#include <vector>
#include <stdio.h>
#include <unordered_map>
#include <typeinfo>
#define MAX_FIRE_VALUE 1000
#define REPAIR_SPEED 1.0f
#define FIRE_DAMAGE_RATE 0.0005f
#define FIRE_SPREAD_MODIFIER .001f
#define FIRE_SPREAD_CHANCE 200
#define MIN_FIRE_VALUE 500
#define DEATH_CHANCE 95*MAX_FIRE_VALUE
#define MAX_OXYGEN 100.0f
#define MIN_OXYGEN_TO_FIRE 20.0f
#define MAX_OXYGEN_SUPPLY_RATE 2.0f
#define MAX_WELD 100.0f
#define OXYGEN_CONSUMPTION 0.00005f
#define FIRE_START_VALUE 10
#define DAMAGE_EFFICENCY_MULTIPLIER .5
#define EXTINGUISH_CONSTANT .5

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

	enum ScenarioResult { Continue, Lost, Won };
	enum DamageState { Operational, Damaged, OutOfAction, Destroyed };

	class Ship
	{
	public:
		std::vector<Point>silvete;
		std::vector<Room*>rooms;
		std::vector<MobileEntity*>mobileEntities;
		Room *findRoom(Point);
		void update();
		Ship();
		~Ship();
	};

	class Door
	{
		std::pair<Room*, Point> firstSide;
		std::pair<Room*, Point> secondSide;
		bool isOpen = false;
		double isWeldedShut = 0.0f;
		bool works = true;
	public:
		std::pair<Room*, Point>otherSide(Room*curr);
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
	};

	class Room
	{
		bool PlayerHasVision;
		double oxygenLevel = MAX_OXYGEN;
		std::vector<StaticEntity>staticEntities;
		std::vector<MobileEntity*>mobileEntities;
		bool onFire = false;
		double fire = 0;
		int sizeX;
		int sizeY;
		std::vector<Door*> doors;
		DamageState state;
		double damageTransition = 100.0f;
		double oxygenGeneration = 0;
		double desiredOxygen = MAX_OXYGEN;
	public:
		std::vector<Door*>::iterator giveDoors();
		std::vector<Door*>::iterator doorsEnd();
		Point findDoor(Room*next);
		bool isOnFire();
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
		std::pair<double, DamageState> currentState();
		bool colides(Point p, MobileEntity* e);
		void setOnFire();
		void setOnFire(double value);
		int fireValue();
		void extinguish(double ammount);
		RoomType whatType();
		int currentOxygenLevel();
		void setDesiredOxygen(double);
		double supplyOxygen(double value);
		double generateOxygen();
		MobileEntity*findEntity(Point);
		MobileEntity*findEntity(Point, MobileEntity*);
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
	public:
		Ship* ship;
		Objective* target;
		std::ofstream& operator<<(std::ofstream&);
		std::ifstream& operator>>(std::ifstream&);
		Scenario(Objective*objective, Ship * vessel);
		Scenario(std::ifstream & file);
		ScenarioResult scenarioTick();
		int ticsRemaining();
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
}