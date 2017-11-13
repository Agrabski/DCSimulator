#pragma once
#include <vector>
#include <unordered_map>
#define REPAIR_SPEED 1.0f
#define FIRE_DAMAGE_RATE 0.0005f
#define FIRE_SPREAD_MODIFIER .001f
#define FIRE_SPREAD_CHANCE 200
#define MIN_FIRE_VALUE 500
#define DEATH_CHANCE 1000
#define MAX_OXYGEN 100.0f
#define MIN_OXYGEN_TO_FIRE 20.0f
#define MAX_OXYGEN_SUPPLY_RATE 2.0f
#define MAX_WELD 100.0f
#define OXYGEN_CONSUMPTION 0.00005f
#define MAX_FIRE_VALUE 1000
#define FIRE_START_VALUE 10

namespace DCS
{
	typedef std::pair<int, int> Point;

	Point operator+(const Point&left, const Point&right);
	Point operator-(const Point&left, const Point&right);
	bool operator<(const Point&left, const Point&right);
	float magnitude(const Point&op);

	class Room;
	class Entity;
	class MobileEntity;
	class StaticEntity;

	enum DamageState {Operational, Damaged, OutOfAction, Destroyed};

	class Ship
	{
	public:
		std::vector<Point>silvete;
		std::vector<Room*>rooms;
		std::vector<MobileEntity*>mobileEntities;
		Room *findRoom(Point);

		Ship();
		~Ship();
	};

	class Door
	{
		std::pair<Room*, Point> firstSide;
		std::pair<Room*, Point> secondSide;
		bool isOpen = false;
		float isWeldedShut = 0.0f;
		bool works = true;
	public:
		std::pair<Room*, Point>otherSide(Room*curr);
		bool weld(float amount);
		void unweld(float amount);
		//returns success state
		bool open();
		//returns success state
		bool close();
		bool isItOpen();
		float isWelded();
		bool isOperational();
		Door(Room*, Point, Room*, Point);
	};

	class Room
	{
		bool PlayerHasVision;
		float oxygenLevel=MAX_OXYGEN;
		std::vector<StaticEntity>staticEntities;
		std::vector<MobileEntity*>mobileEntities;
		bool onFire = false;
		double fire = 0;
		int sizeX;
		int sizeY;
		Room*up;
		Point positionUp;
		Room*down;
		Point positionDown;
		Room*left;
		Point positionLeft;
		Room*right;
		Point positionRight;
		DamageState state;
		float damageTransition=100.0f;
	public:
		bool isOnFire();
		Point position;
		std::vector<Point>silvete;
		enum RoomType { Bridge, Engineering, Corridor, LifeSupport };
		void update();
		Room(Point position, std::vector<Point>Silvete, std::vector<StaticEntity>entities, RoomType type);
		void setUp(Room*, Point);
		void setDown(Room*, Point);
		void setLeft(Room*, Point);
		void setRight(Room*, Point);
		void removeEntity(MobileEntity*e);
		void addEntity(MobileEntity*e);
		std::pair<Room*, Point>leftDoor();
		std::pair<Room*, Point>rightDoor();
		std::pair<Room*, Point>downDoor();
		std::pair<Room*, Point>upDoor();
		void damage(float damage);
		void repair(float amount);
		std::pair<float,DamageState> currentState();
		bool colides(Point p, MobileEntity* e);
		void setOnFire();
		void setOnFire(double value);
		int fireValue();
		void extinguish(float ammount);
		RoomType whatType();
		int currentOxygenLevel();
	private:
		RoomType type;

	};

	class Entity
	{
	public:
		//returns pointer to the current room
		virtual Room* update()=0;
		Point position;
		Room*currentRoom;
	protected:


	};

	class MobileEntity:public Entity
	{
	public:
		enum Status { Nominal,Wounded,Dead };
	private:
		Status health = Nominal;
		std::vector<Room*>path;
		Point findDoor(Room*next);
		void findPath();
		int findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * destination, DCS::Point door, DCS::Point position, std::unordered_map<Room*, std::pair<DCS::Point, int>>*);
	public:
		Status currentStatus();
		bool selected = false;
		//returns pointer to the current room
		virtual Room* update();
		enum MobileEntityType {Marine,Engineer,CrewMember,Boarder}type;
		MobileEntity(Room*current, Point location, MobileEntityType type);
		std::pair<Point, Room*> destination;
		Room*location();
		void changeDestination(std::pair<Point, Room*> d);
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

	public:
		std::vector<MobileEntity*>selected;
		Ship ship;
		void gameTick();
	};
}