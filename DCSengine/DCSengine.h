#pragma once
#include <vector>


namespace DCS
{
	typedef std::pair<int, int> Point;

	Point operator+(const Point&left, const Point&right);

	class Room;
	class Entity;
	class MobileEntity;
	class StaticEntity;

	class Ship
	{
	public:
		std::vector<Point>silvete;
		std::vector<Room*>rooms;
		std::vector<MobileEntity*>mobileEntities;

		Ship();
	};

	class Room
	{
		bool PlayerHasVision;

		std::vector<StaticEntity>staticEntities;
		std::vector<MobileEntity*>mobileEntities;
		Point position;
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
	public:
		std::vector<Point>silvete;
		enum RoomType { Bridge, Engineering, Corridor, };
		void update();
		Room(Point position, std::vector<Point>Silvete, std::vector<StaticEntity>entities, RoomType type);
		void setUp(Room*, Point);
		void setDown(Room*, Point);
		void setLeft(Room*, Point);
		void setRight(Room*, Point);
	private:
		RoomType type;

	};

	class Entity
	{
	public:
		//returns pointer to the current room
		virtual Room* update()=0;
	protected:
		Room*currentRoom;
		Point position;
	};

	class MobileEntity:public Entity
	{
	public:
		//returns pointer to the current room
		virtual Room* update();
		enum MobileEntityType {Marine,Engineer,CrewMember,Boarder}type;

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
		int x = 0;
		int y = 0;
		Ship ship;
		std::vector<MobileEntity*>selected;
	public:
		void gameTick();
		int targetX = 999999999;
		int targetY = 999999999;
	};
}