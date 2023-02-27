#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Scene.h"
#include "Target.h"
#include "StarShip.h"
#include "Tile.h"
#include "Heuristic.h"
#include "Obstacle.h"
#include <queue>
#include "TileComparator.h"

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene() override;

	// Scene LifeCycle Functions
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;
	virtual void HandleEvents() override;
	virtual void Start() override;
private:
	// IMGUI Function
	void GUI_Function();
	std::string m_guiTitle;
	glm::vec2 m_mousePosition;

	// Game Objects for the Scene
	Target* m_pTarget;
	StarShip* m_pStarShip;

	// Debugging Variables
	bool m_bDebugView;

	// Pathfinding Objects and Functions
	std::vector<Tile*> m_pGrid;
	bool m_isGridEnabled;

	void m_buildGrid();
	bool m_getGridEnabled() const;
	void m_setGridEnabled(bool state);
	void m_computeTileCosts();

	// obstacle list
	std::vector<Obstacle*> m_pObstacles;
	void m_buildObstacles();

	//utiltiy
	void m_removeObstacleAt(int col, int row);
	void m_removeObstacleAt(glm::vec2 grid_position);
	void m_removeAllObstacles();


	// tile lists for pathfinding
	std::priority_queue<Tile*, std::vector<Tile*>, TileComparator> m_pOpenList;
	std::list<Tile*> m_pPathList;
	bool m_pathFound = false;


	//std::vector<Tile*> m_pClosedList;

	// calculate shortest path functions
	void m_findShortestPath();
	void m_buildPathList();
	void m_displayPathList();
	void m_resetPathFinding();
	void m_resetSimulation();

	
	// TODO: some kind of data structure for the path list
	// suggestions: a queue data structure or a linked list

	// convenience Functions to convert world to grid space
	Tile* m_getTile(int col, int row) const;
	Tile* m_getTile(glm::vec2 grid_position) const;

	template <typename T>
	void m_addObjectToGrid(T*& object, int col, int row, TileStatus status);


	template <typename T>
	void m_moveGameObject(T*& object, int col, int row, TileStatus status);




	//void m_markImpassable();

	// heuristic
	Heuristic m_currentHeuristic;

	//tile map
	std::string m_tileMap;
	void m_initializeTileMap();
	void m_buildTileMap();

	//convenience functions
	void m_updateTileMap(int col, int row, TileStatus status);
	void m_updateTileMap(glm::vec2 grid_position, TileStatus status);


};

#endif /* defined (__PLAY_SCENE__) */