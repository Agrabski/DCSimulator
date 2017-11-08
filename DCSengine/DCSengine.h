#pragma once
#include <vector>
#include <unordered_map>


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
	};

	class Room
	{
		bool PlayerHasVision;

		std::vector<StaticEntity>staticEntities;
		std::vector<MobileEntity*>mobileEntities;

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

		std::vector<Room*>path;
		Point findDoor(Room*next);
		void findPath();
		int findRoute(std::pair<std::vector<DCS::Room*>, int>& currPath, int currentBest, DCS::Room * location, DCS::Room * destination, DCS::Point door, DCS::Point position, std::unordered_map<Room*, std::pair<DCS::Point, int>>*);
	public:
		bool selected = false;
		//returns pointer to the current room
		virtual Room* update();
		enum MobileEntityType {Marine,Engineer,CrewMember,Boarder}type;
		MobileEntity(Room*current, Point location, MobileEntityType type);
		std::pair<Point, Room*> destination;
		Room*location();
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