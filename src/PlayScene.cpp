#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"

PlayScene::PlayScene()
{
	PlayScene::Start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::Draw()
{
	DrawDisplayList();

	SDL_SetRenderDrawColor(Renderer::Instance().GetRenderer(), 255, 255, 255, 255);
}

void PlayScene::Update()
{
	UpdateDisplayList();
}

void PlayScene::Clean()
{
	RemoveAllChildren();
}

void PlayScene::HandleEvents()
{
	EventManager::Instance().Update();

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_ESCAPE))
	{
		Game::Instance().Quit();
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_1))
	{
		Game::Instance().ChangeSceneState(SceneState::START);
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_2))
	{
		Game::Instance().ChangeSceneState(SceneState::END);
	}
}

template <typename T>
T* PlayScene::m_addNavigationObjectToGrid(T* object, const int col, const int row, const TileStatus status)
{
	const auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	object = new T();
	object->GetTransform()->position = m_getTile(col, row)->GetTransform()->position + offset;
	object->SetGridPosition(static_cast<float>(col), static_cast<float>(row));
	m_getTile(col, row)->SetTileStatus(status);
	AddChild(object);
	return object;
}

void PlayScene::m_markImpassable()
{
	m_addNavigationObjectToGrid(m_pObstacleList[0], 9, 0, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[1], 9, 1, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[2], 9, 2, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[3], 9, 3, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[4], 9, 4, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[5], 9, 5, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[6], 9, 6, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[7], 9, 7, TileStatus::IMPASSABLE);

	m_addNavigationObjectToGrid(m_pObstacleList[8], 5, 6, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[9], 5, 7, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[10], 5, 8, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[11], 5, 9, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[12], 5, 10, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[13], 5, 11, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[14], 5, 12, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[15], 5, 13, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[16], 5, 14, TileStatus::IMPASSABLE);

	m_addNavigationObjectToGrid(m_pObstacleList[17], 13, 7, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[18], 13, 8, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[19], 13, 9, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[20], 13, 10, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[21], 13, 11, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[22], 13, 12, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[23], 13, 13, TileStatus::IMPASSABLE);
	m_addNavigationObjectToGrid(m_pObstacleList[24], 13, 14, TileStatus::IMPASSABLE);
}

void PlayScene::Start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";

	m_buildGrid();
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	m_currentHeuristic = Heuristic::MANHATTAN;

	// Add the Target to the Scene
	m_pTarget = m_addNavigationObjectToGrid(m_pTarget, 15, 11, TileStatus::GOAL);

	// Add the StarShip to the Scene
	m_pStarShip = m_addNavigationObjectToGrid(m_pStarShip, 1, 3, TileStatus::START);

	m_markImpassable();

	// Preload Sounds

	SoundManager::Instance().Load("../Assets/Audio/yay.ogg", "yay", SoundType::SOUND_SFX);
	SoundManager::Instance().Load("../Assets/Audio/thunder.ogg", "thunder", SoundType::SOUND_SFX);

	m_computeTileCosts();

	ImGuiWindowFrame::Instance().SetGuiFunction(std::bind(&PlayScene::GUI_Function, this));
}

void PlayScene::GUI_Function()
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();
	
	ImGui::Begin("GAME3001 - W2023 - Lab4", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar );

	ImGui::Separator();

	// Debug Properties
	static bool toggle_grid = false;
	if(ImGui::Checkbox("Toggle Grid", &toggle_grid))
	{
		m_isGridEnabled = toggle_grid;
		m_setGridEnabled(m_isGridEnabled);
	}

	ImGui::Separator();

	static int radio = static_cast<int>(m_currentHeuristic);
	ImGui::Text("Heuristic Type");
	ImGui::RadioButton("Manhattan", &radio, static_cast<int>(Heuristic::MANHATTAN));
	ImGui::SameLine();
	ImGui::RadioButton("Euclidean", &radio, static_cast<int>(Heuristic::EUCLIDEAN));

	// check if  current heuristic is not the same as the selection
	if(m_currentHeuristic != static_cast<Heuristic>(radio))
	{
		m_currentHeuristic = static_cast<Heuristic>(radio);
		m_computeTileCosts();
	}

	ImGui::Separator();

	// StarShip Properties
	static int start_position[2] = {
		static_cast<int>(m_pStarShip->GetGridPosition().x),
		static_cast<int>(m_pStarShip->GetGridPosition().y) };
	if(ImGui::SliderInt2("Start Position", start_position, 0, Config::COL_NUM - 1))
	{
		// constrain the object within max rows
		if(start_position[1] > Config::ROW_NUM - 1)
		{
			start_position[1] = Config::ROW_NUM - 1;
		}

		// convert grid space to world space
		m_getTile(m_pStarShip->GetGridPosition())->SetTileStatus(TileStatus::UNVISITED);
		m_pStarShip->GetTransform()->position = 
			m_getTile(start_position[0], start_position[1])->GetTransform()->position + offset;
		m_pStarShip->SetGridPosition(start_position[0], start_position[1]);
		m_getTile(m_pStarShip->GetGridPosition())->SetTileStatus(TileStatus::START);
	}

	ImGui::Separator();

	// Target Properties
	static int goal_position[2] = {
		static_cast<int>(m_pTarget->GetGridPosition().x),
		static_cast<int>(m_pTarget->GetGridPosition().y) };
	if (ImGui::SliderInt2("Goal Position", goal_position, 0, Config::COL_NUM - 1))
	{
		// constrain the object within max rows
		if (goal_position[1] > Config::ROW_NUM - 1)
		{
			goal_position[1] = Config::ROW_NUM - 1;
		}

		// convert grid space to world space
		m_getTile(m_pTarget->GetGridPosition())->SetTileStatus(TileStatus::UNVISITED);
		m_pTarget->GetTransform()->position =
			m_getTile(goal_position[0], goal_position[1])->GetTransform()->position + offset;
		m_pTarget->SetGridPosition(goal_position[0], goal_position[1]);
		m_getTile(m_pTarget->GetGridPosition())->SetTileStatus(TileStatus::GOAL);
		m_computeTileCosts();
	}

	ImGui::End();
}

void PlayScene::m_buildGrid()
{
	const auto tile_size = Config::TILE_SIZE;

	// layout a grid of tiles
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = new Tile();
			// world space position
			tile->GetTransform()->position = glm::vec2(col * tile_size, row * tile_size);
			tile->SetGridPosition(col, row); // Grid Position
			tile->SetParent(this);
			tile->AddLabels();
			AddChild(tile);
			tile->SetEnabled(false);
			m_pGrid.push_back(tile);
		}
	}

	// setup the neighbour references for each tile in the grid
	// tiles = nodes in our graph

	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = m_getTile(col, row);

			// TopMost Neighbour
			if(row == 0)
			{
				tile->SetNeighbourTile(NeighbourTile::TOP_TILE, nullptr);
			}
			else
			{
				tile->SetNeighbourTile(NeighbourTile::TOP_TILE, m_getTile(col, row - 1));
			}

			// RightMost Neighbour
			if(col == Config::COL_NUM - 1)
			{
				tile->SetNeighbourTile(NeighbourTile::RIGHT_TILE, nullptr);
			}
			else
			{
				tile->SetNeighbourTile(NeighbourTile::RIGHT_TILE, m_getTile(col + 1,row));
			}

			// BottomMost Neighbour
			if(row == Config::ROW_NUM - 1)
			{
				tile->SetNeighbourTile(NeighbourTile::BOTTOM_TILE, nullptr);
			}
			else
			{
				tile->SetNeighbourTile(NeighbourTile::BOTTOM_TILE, m_getTile(col, row + 1));
			}

			// LeftMost Neighbour
			if (col == 0)
			{
				tile->SetNeighbourTile(NeighbourTile::LEFT_TILE, nullptr);
			}
			else
			{
				tile->SetNeighbourTile(NeighbourTile::LEFT_TILE, m_getTile(col - 1, row));
			}
		}
	}
}

bool PlayScene::m_getGridEnabled() const
{
	return m_isGridEnabled;
}

void PlayScene::m_setGridEnabled(const bool state)
{
	m_isGridEnabled = state;

	for (const auto tile : m_pGrid)
	{
		tile->SetEnabled(m_isGridEnabled); // enables each tile object
		tile->SetLabelsEnabled(m_isGridEnabled); // enables the corresponding labels
	}
}

void PlayScene::m_computeTileCosts()
{
	float distance = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;

	// for each tile in the grid, loop
	for (const auto tile : m_pGrid)
	{
		// compute the distance from each tile to the goal tile
		// distance (f) = tile cost (g) + heuristic estimate (h)
		switch(m_currentHeuristic)
		{
		case Heuristic::MANHATTAN:
			dx = abs(tile->GetGridPosition().x - m_pTarget->GetGridPosition().x);
			dy = abs(tile->GetGridPosition().y - m_pTarget->GetGridPosition().y);
			distance = dx + dy;
			break;
		case Heuristic::EUCLIDEAN:
			// calculates the euclidean distance ("as the crow flies") for each tile
			distance = Util::Distance(tile->GetGridPosition(), m_pTarget->GetGridPosition());
			break;
		}

		tile->SetTileCost(distance);
	}
}

void PlayScene::m_findShortestPath()
{
	
}

void PlayScene::m_displayPathList()
{
}

void PlayScene::m_resetPathFinding()
{
}

void PlayScene::m_resetSimulation()
{
}

Tile* PlayScene::m_getTile(const int col, const int row) const
{
	return m_pGrid[(row * Config::COL_NUM) + col];
}

Tile* PlayScene::m_getTile(const glm::vec2 grid_position) const
{
	const auto col = grid_position.x;
	const auto row = grid_position.y;

	return m_getTile(static_cast<int>(col), static_cast<int>(row));
}
