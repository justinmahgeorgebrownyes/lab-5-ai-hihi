#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"
#include <Windows.h>

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
void PlayScene::m_addObjectToGrid(T*& object, const int col, const int row, const TileStatus status)
{
	const auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	object = new T();
	object->GetTransform()->position = m_getTile(col, row)->GetTransform()->position + offset;
	object->SetGridPosition(static_cast<float>(col), static_cast<float>(row));
	m_getTile(col, row)->SetTileStatus(status);
	AddChild(object);
	//return object;
}


template <typename T>
void PlayScene::m_moveGameObject(T*& object, int col, int row, TileStatus status) {

	constexpr auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);


	if (m_getTile(object->GetGridPosition())->GetTileStatus() != TileStatus::IMPASSABLE) {

		m_getTile(object->GetGridPosition())->SetTileStatus(TileStatus::UNVISITED);
		m_updateTileMap(object->GetGridPosition(), TileStatus::UNVISITED);

	}

	m_pTarget->GetTransform()->position =
		m_getTile(col,row)->GetTransform()->position + offset;
	m_pTarget->SetGridPosition(static_cast<float>(col), static_cast<float>(row));

	if (m_getTile(object->GetGridPosition())->GetTileStatus() != TileStatus::IMPASSABLE) {
		m_getTile(object->GetGridPosition())->SetTileStatus(status);

		m_updateTileMap(col, row, status);

	}

	if (!m_pPathList.empty()) {

		m_resetPathFinding();

	}
	//m_computeTileCosts();


}

void PlayScene::Start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";


	m_buildObstacles();



	m_buildGrid();
	//auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	m_currentHeuristic = Heuristic::MANHATTAN;

	m_initializeTileMap();
	m_buildTileMap();

	m_computeTileCosts();
	//m_findShortestPath();
	// Add the StarShip to the Scene
	//m_addObjectToGrid(m_pStarShip, 1, 3, TileStatus::START);

	// Add the Target to the Scene
	//m_addObjectToGrid(m_pTarget, 15, 11, TileStatus::GOAL);

	

	//m_markImpassable();

	// Preload Sounds

	SoundManager::Instance().Load("../Assets/Audio/yay.ogg", "yay", SoundType::SOUND_SFX);
	SoundManager::Instance().Load("../Assets/Audio/thunder.ogg", "thunder", SoundType::SOUND_SFX);

	m_computeTileCosts();

	ImGuiWindowFrame::Instance().SetGuiFunction(std::bind(&PlayScene::GUI_Function, this));
}

void PlayScene::GUI_Function()
{
	//auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

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


	if (ImGui::CollapsingHeader("Tile Map")) {

		static char* tile_map[300];

		for (int i = 0; i < m_tileMap.length(); ++i) {

			tile_map[i] = new char[i];
			sprintf(tile_map[i], "%c + %d", m_tileMap[i], i);

			if ((i % 20) != 0) {


				ImGui::SameLine();

			}

			if (m_tileMap[i] == 'S') {


				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.360f, 0.721f, 0.360f, 1.0f)); //green


			}
			else if (m_tileMap[i] == 'G') {

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.850f, 0.325f, 0.309f, 1.0f)); //red


			}
			else {

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.007f, 0.458f, 0.847f, 1.0f)); //blue

			}

			if (ImGui::Button(tile_map[i], ImVec2(15, 15)))
			{
				if (m_tileMap[i] != 'S' && m_tileMap[i] != 'G')
				{
					m_tileMap[i] = (m_tileMap[i] == '-') ? 'I' : '-';
					m_removeAllObstacles();
					m_buildObstacles();
					m_buildTileMap();
					m_resetPathFinding();
				}
			}

			ImGui::PopStyleColor();


		}

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

		if (!m_pPathList.empty()) {

			m_resetPathFinding();

		}


		m_computeTileCosts();
	}

	ImGui::Separator();


	if (ImGui::Button("find shortest path")) {

		if (!m_pPathList.empty()) {

			m_resetPathFinding();

		}
		m_findShortestPath();


	}

	if (m_pathFound) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Path Found! - Length: %d", (m_pPathList.size() - 1));
	}

	ImGui::Separator();

	//position variable
	static int start_position[2] = {
	static_cast<int>(m_pStarShip->GetGridPosition().x),
	static_cast<int>(m_pStarShip->GetGridPosition().y) };

	static int goal_position[2] = {
	static_cast<int>(m_pTarget->GetGridPosition().x),
	static_cast<int>(m_pTarget->GetGridPosition().y) };


	if (ImGui::Button("Reset Path Finding")) {

		m_resetPathFinding();

	}

	ImGui::SameLine();
	if (ImGui::Button("Reset simulation")) {
		m_resetSimulation();
		start_position[0] = static_cast<int>(m_pStarShip->GetGridPosition().x);
		start_position[1] = static_cast<int>(m_pStarShip->GetGridPosition().y);
		start_position[0] = static_cast<int>(m_pTarget->GetGridPosition().x);
		start_position[1] = static_cast<int>(m_pTarget->GetGridPosition().y);

	}


	ImGui::Separator();



	// StarShip Properties

	if(ImGui::SliderInt2("Start Position", start_position, 0, Config::COL_NUM - 1))
	{
		// constrain the object within max rows
		if(start_position[1] > Config::ROW_NUM - 1)
		{
			start_position[1] = Config::ROW_NUM - 1;
		}


		m_moveGameObject(m_pTarget, start_position[0], start_position[1], TileStatus::GOAL);


	}

	ImGui::Separator();

	// Target Properties

	if (ImGui::SliderInt2("Goal Position", goal_position, 0, Config::COL_NUM - 1))
	{
		// constrain the object within max rows
		if (goal_position[1] > Config::ROW_NUM - 1)
		{
			goal_position[1] = Config::ROW_NUM - 1;
		}


		m_moveGameObject(m_pTarget, goal_position[0], goal_position[1], TileStatus::GOAL);

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

void PlayScene::m_buildObstacles() {

	for (int i = 0; i < 300; ++i) {
		m_pObstacles.push_back(new Obstacle());
	}


}

void PlayScene::m_removeObstacleAt(int col, int row)
{

	for (auto& obstacle : m_pObstacles) {

		if (obstacle != nullptr) {


			if (static_cast<int>(obstacle->GetGridPosition().x) == col && static_cast<int>(obstacle->GetGridPosition().y) == row) {
				RemoveChild(obstacle);
				m_pObstacles[(row * Config::COL_NUM) + col] = new Obstacle();
			}

		}

	}


}

void PlayScene::m_removeObstacleAt(glm::vec2 grid_position)
{
	m_removeObstacleAt(static_cast<int>(grid_position.x), static_cast<int>(grid_position.y));



}

void PlayScene::m_removeAllObstacles()
{
	for (auto& obstacle : m_pObstacles) {

		RemoveChild(obstacle);
	}
	m_pObstacles.clear();


}

void PlayScene::m_findShortestPath()
{
	m_getTile(m_pStarShip->GetGridPosition())->SetTileParent(nullptr);
	m_pOpenList.push(m_getTile(m_pStarShip->GetGridPosition()));
	bool goal_found = false;

	while (!m_pOpenList.empty() && !goal_found) {

		const auto current_node = m_pOpenList.top();
		std::cout << current_node->GetGridPosition().x << ", " << current_node->GetGridPosition().y << std::endl;

		m_pOpenList.pop();
		current_node->SetTileStatus(TileStatus::CLOSED);

		for (auto neighbour : current_node->GetNeighbourTiles()) {

			if (neighbour == nullptr || neighbour->GetTileStatus() == TileStatus::IMPASSABLE) {

				continue;


			}
			if (neighbour->GetTileStatus() == TileStatus::GOAL) {

				goal_found = true;
				neighbour->SetTileParent(current_node);
				std::cout << "Goal Found at: " << neighbour->GetGridPosition().x << ", " << neighbour->GetGridPosition().y << std::endl;
				m_pPathList.push_front(neighbour);
				break;



			}
			if (neighbour->GetTileStatus() == TileStatus::UNVISITED) {

				neighbour->SetTileParent(current_node);
				neighbour->SetTileStatus(TileStatus::OPEN);
				m_pOpenList.push(neighbour);

			}


		}



	}

	if (goal_found) {

		m_pathFound = true;
		m_buildPathList();
		m_displayPathList();
	}
	else {

		const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, 4); // red
		std::cout << "No Path Found!" << std::endl;
		SetConsoleTextAttribute(handle, 15); // red

	}


}

void PlayScene::m_buildPathList()
{
	while (m_pPathList.front()->GetTileParent() != nullptr) {

		m_pPathList.push_front(m_pPathList.front()->GetTileParent());

	}


}

void PlayScene::m_displayPathList()
{
	std::cout << "Total Path Length is: " << m_pPathList.size() - 1 << std:: endl;
	std::cout << "Path LIst is :" << std::endl;
	std::cout << "===============" << std::endl;
	for (const auto node : m_pPathList) {
		node->SetTileStatus(TileStatus::PATH);
		std::cout << node->GetGridPosition().x << ", " << node->GetGridPosition().y << std::endl;
	}


}

void PlayScene::m_resetPathFinding()
{
	while (!m_pOpenList.empty()) {
		m_pOpenList.pop();
	}

	while (!m_pPathList.empty()) {
		m_pPathList.pop_front();
	}

	m_pathFound = false;

	for (const auto tile : m_pGrid) {

		if (tile->GetTileStatus() == TileStatus::OPEN 
			|| tile->GetTileStatus() == TileStatus::CLOSED 
			|| tile->GetTileStatus() == TileStatus::PATH) {

			tile->SetTileStatus(TileStatus::UNVISITED);


		}

	}


	m_getTile(m_pStarShip->GetGridPosition())->SetTileStatus(TileStatus::START);
	m_getTile(m_pTarget->GetGridPosition())->SetTileStatus(TileStatus::GOAL);


}

void PlayScene::m_resetSimulation()
{
	m_removeAllObstacles();
	m_buildObstacles();
	RemoveChild(m_pStarShip);
	RemoveChild(m_pTarget);
	m_initializeTileMap();
	m_buildTileMap();
	m_computeTileCosts();
	m_resetPathFinding();

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

void PlayScene::m_initializeTileMap()
{
	m_tileMap = "---------I----------";
	m_tileMap += "---------I----------";
	m_tileMap += "---------I----------";
	m_tileMap += "--S------I----------";
	m_tileMap += "---------I----------";
	m_tileMap += "---------I----------";
	m_tileMap += "-----I---I---I------";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I-G----";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I------";
	m_tileMap += "-----I-------I------";

}

void PlayScene::m_buildTileMap()
{
	for (int row = 0; row < Config::ROW_NUM; ++row) {
		for (int col = 0; col < Config::COL_NUM; ++col) {

			if (m_tileMap[(row * Config::COL_NUM) + col] == 'I') {

				m_addObjectToGrid(m_pObstacles[(row * Config::COL_NUM) + col], col, row, TileStatus::IMPASSABLE);
			}
			if (m_tileMap[(row * Config::COL_NUM) + col] == 'S') {

				m_addObjectToGrid(m_pStarShip, col, row, TileStatus::START);
			}
			if (m_tileMap[(row * Config::COL_NUM) + col] == 'G') {

				m_addObjectToGrid(m_pTarget, col, row, TileStatus::GOAL);
			}
			if (m_tileMap[(row * Config::COL_NUM) + col] == '-') {

				m_getTile(col, row) ->SetTileStatus(TileStatus::UNVISITED);
				//m_addObjectToGrid(m_pStarShip, col, row, TileStatus::START);
			}
			
		}
	}


}

void PlayScene::m_updateTileMap(int col, int row, TileStatus status)
{

	switch (status) {
	case TileStatus::UNVISITED:
		m_tileMap[(row * Config::COL_NUM) + col] = '-';
			break;
	
	case TileStatus::OPEN:
		m_tileMap[(row * Config::COL_NUM) + col] = 'O';
			break;
	
	case TileStatus::CLOSED:
		m_tileMap[(row * Config::COL_NUM) + col] = 'C';
			break;

	case TileStatus::GOAL:
		m_tileMap[(row * Config::COL_NUM) + col] = 'G';
		break;

	case TileStatus::START:
		m_tileMap[(row * Config::COL_NUM) + col] = 'S';
		break;

	case TileStatus::IMPASSABLE:
		m_tileMap[(row * Config::COL_NUM) + col] = 'I';
		break;
	
	case TileStatus::PATH:
		m_tileMap[(row * Config::COL_NUM) + col] = 'P';
		break;
	

	}

}

void PlayScene::m_updateTileMap(glm::vec2 grid_position, TileStatus status)
{

	m_updateTileMap(static_cast<int>(grid_position.x), static_cast<int>(grid_position.y), status);

}
