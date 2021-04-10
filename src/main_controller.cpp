
#include "main_controller.h"
#include <iostream>
#include "render.h"
#include "render_tank0.h"
#include "render_left_panel_info.h"
#include "utils_loader_screen.h"
#include "utils_start_dialog.h"
#include "json.hpp"
#include <fstream>
#include "render_player_alient_ship.h"
#include "wsjcpp_core.h"



// MainController

MainController::MainController(const std::string &sWindowName) {
    m_sWindowName = sWindowName;
    m_nWindowWidth = 1280;
    m_nWindowHeight = 720;
    m_pRenderWindow = nullptr;
    m_nProgressBarStatus = 0;
    m_nProgressBarMax = 100;
    m_sResourceDir = "./res";
    m_pAlientShipState = nullptr;
    TAG = "MainController";
}

bool MainController::findResourceDir() {
    std::vector<std::string> vPaths;
    vPaths.push_back("/usr/share/yourCityIsInvadedByAliens_Tomsk");
    vPaths.push_back("./res");
    m_sResourceDir = "";
    for (int i = 0; i < vPaths.size(); i++) {
        if (WsjcppCore::dirExists(vPaths[i])) {
            m_sResourceDir = vPaths[i];
            break;
        }
    }
    if (m_sResourceDir == "") {
        std::cerr << "Not found resource dir (./res by default)" << std::endl;
        return false;
    }
    std::cout << "Resource dir: " << m_sResourceDir << std::endl;
    return true;
}

std::string MainController::getResourceDir() {
    return m_sResourceDir;
}

bool MainController::initSDL2() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        std::cerr << "HEY.. SDL_Init HAS FAILED. SDL_ERROR: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG))) {
        std::cerr << "IMG_init has failed. Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return false;
    }
    
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }
    int result = 0;
    int flags = MIX_INIT_MP3;
    if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }
    return true;
}

bool MainController::initRenderWindow() {
    
    if (!this->initSDL2()) {
        return false;
    }

    m_pRenderWindow = new RenderWindow(
        m_sWindowName.c_str(),
        m_nWindowWidth,
        m_nWindowHeight
    );
    m_pGameState = new GameState(
        m_nWindowWidth,
        m_nWindowHeight
    );
    return true;
}

RenderWindow *MainController::getWindow() {
    return m_pRenderWindow;
}

GameState *MainController::getGameState() {
    return m_pGameState;
}

CoordXY MainController::getCoordCenter() {
    return CoordXY(
        m_nWindowWidth/2,
        m_nWindowHeight/2
    );
}

bool MainController::loadGameDataWithProgressBar() {
    UtilsLoaderScreen loader(
        m_sResourceDir,
        m_pRenderWindow,
        m_pGameState
    );
    loader.init();
    loader.updateText("Loading... textures");

    m_pTextureBackground = m_pRenderWindow->loadTexture(m_sResourceDir + "/default/textures/background.png");
    m_pTextureAlienShip1 = m_pRenderWindow->loadTexture(m_sResourceDir + "/sprites/alien-ship.png");
    m_pTextureTank0 = m_pRenderWindow->loadTexture(m_sResourceDir + "/sprites/tank0.png");
    m_pRenderWindow->loadTextureRocket(m_sResourceDir + "/sprites/tank0-rocket.png");
    m_pTextureLeftPanel = m_pRenderWindow->loadTexture(m_sResourceDir + "/app/textures/left-panel-info.png");
    m_pRenderWindow->loadTextureBioplast(m_sResourceDir + "/sprites/alient-bioplast.png");

    loader.updateText("Loading... buildings textures");

    std::vector<std::string> vBuildings = WsjcppCore::getListOfDirs(m_sResourceDir + "/buildings");
    for (int i = 0; i < vBuildings.size(); i++) {
        std::string sName = vBuildings[i];
        std::string sPathTexture = m_sResourceDir + "/buildings/" + sName + "/texture.png";
        if (!WsjcppCore::fileExists(sPathTexture)) {
            WsjcppLog::err(TAG, "Not found " + sPathTexture);
            loader.updateText("Not found " + sPathTexture);
            return false;
        }
        m_mapBuildingsTextures[sName] = m_pRenderWindow->loadTexture(sPathTexture.c_str());
    }

    loader.updateText("Loading... buildings");
    
    SDL_Texture* pTextureTower0 = m_mapBuildingsTextures["tower0"];

    // load map from json
    std::ifstream ifs(m_sResourceDir + "/data.json");
    nlohmann::json jf = nlohmann::json::parse(ifs);
    
    nlohmann::json jsonBuildings = jf["buildings"];
    for (auto it = jsonBuildings.begin(); it != jsonBuildings.end(); ++it) {
        // std::cout << it.key() << " | " << it.value() << "\n";
        GameBuilding *pBuilding = new GameBuilding(it.value());
        m_pGameState->addBuilding(pBuilding);
        RenderBuilding2 *pRenderBuilding2 = new RenderBuilding2(pBuilding, pTextureTower0);
        CoordXY min0 = pRenderBuilding2->getMinPoint();
        m_minPointMap.update(
            std::min(min0.x(), m_minPointMap.x()),
            std::min(min0.y(), m_minPointMap.y())
        );
        CoordXY max0 = pRenderBuilding2->getMaxPoint();
        m_maxPointMap.update(
            std::max(max0.x(), m_maxPointMap.x()),
            std::max(max0.y(), m_maxPointMap.y())
        );
        m_pRenderWindow->addObject(pRenderBuilding2);
    }

    // background
    // around to 500px
    m_minPointMap.update(
        m_minPointMap.x() - ((m_minPointMap.x() - 500) % 500) - 500,
        m_minPointMap.y() - ((m_minPointMap.y() - 500) % 500) - 500
    );
    m_maxPointMap.update(
        m_maxPointMap.x() - ((m_maxPointMap.x() + 500) % 500) + 500,
        m_maxPointMap.y() - ((m_maxPointMap.y() + 500) % 500) + 500
    );
    for (int x = m_minPointMap.x(); x <= m_maxPointMap.x(); x += 500) {
        for (int y = m_minPointMap.y(); y <= m_maxPointMap.y(); y += 500) {
            m_pRenderWindow->addObject(new RenderBackground(CoordXY(x, y), m_pTextureBackground, -10));        
        }
    }

    m_pGameState->setMinPoint(m_minPointMap);
    m_pGameState->setMaxPoint(m_maxPointMap);
    
    loader.updateText("Generating enimies...");
    this->generateTanks();

    m_pRenderWindow->addObject(
        new RenderLeftPanelInfo(m_pTextureLeftPanel, 5000)
    );

    // text
    m_pFpsText = new RenderAbsoluteTextBlock(CoordXY(m_nWindowWidth - 270, 20), "FPS: ...", 5001);
    m_pRenderWindow->addObject(m_pFpsText);

    // coordinates of player
    m_pCoordText = new RenderAbsoluteTextBlock(CoordXY(m_nWindowWidth - 270, 40), "x = ? y = ?", 5001);
    m_pRenderWindow->addObject(m_pCoordText);

    m_pAlientShipState = new GameAlienShipState(getCoordCenter());
    m_pRenderWindow->addObject(
        new RenderPlayerAlienShip(
            m_pAlientShipState,
            m_pTextureAlienShip1,
            1000
        )
    );
    return true;
}

bool MainController::showStartDialog() {
     UtilsStartDialog dialog(
        m_sResourceDir,
        m_pRenderWindow,
        m_pGameState
    );
    dialog.init();
    return dialog.start();
}

bool MainController::isFullscreen() {
    return m_pRenderWindow->isFullscreen();
}

void MainController::toggleFullscreen() {
    m_pRenderWindow->toggleFullscreen();
    int w,h;
    m_pRenderWindow->getWindowSize(&w,&h);
    std::cout << "w,h = (" << w << "," << h << ")";
    m_pGameState->updateWindowSize(w,h);
    m_nWindowWidth = w;
    m_nWindowHeight = h;
}

void MainController::modifyObjects() {
    m_pRenderWindow->modifyObjects(*m_pGameState);

    CoordXY p0 = m_pAlientShipState->getPosition();
    // calculate intersection rockets and player
    for (int i = 0; i < m_pRenderWindow->m_vRockets.size(); i++) {
        GameRocketState *pRocket = m_pRenderWindow->m_vRockets[i];
        if (pRocket->isExploded() || pRocket->hasDestroyed()) {
            continue;
        }
        CoordXY p1 = pRocket->getPosition();
         // distance
        double dx = p0.x() - p1.x();
        double dy = p0.y() - p1.y();
        double nDistance = sqrt(dx * dx + dy * dy);
        if (nDistance < 20.0) {
            pRocket->explode();
            m_pAlientShipState->rocketAttack();
        }
    }

    for (int i = 0; i < m_pRenderWindow->m_vRockets.size(); i++) {
        GameRocketState *pRocket = m_pRenderWindow->m_vRockets[i];
        if (pRocket->isExploded() || pRocket->hasDestroyed()) {
            continue;
        }
        CoordXY p1 = pRocket->getPosition();

        for (int b = 0; b < m_pRenderWindow->m_vBioplasts.size(); b++) {
            GameBioplastState *pBioplast = m_pRenderWindow->m_vBioplasts[b];
            CoordXY p2 = pBioplast->getPosition();
            // distance
            double dx = p1.x() - p2.x();
            double dy = p1.y() - p2.y();
            double nDistance = sqrt(dx * dx + dy * dy);
            if (nDistance < 15.0) {
                pRocket->explode();
                // m_pAlientShipState->rocketAttack();
            }
        }
    }

}

bool MainController::isKeyboardUp(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        keyboard_state_array[SDL_SCANCODE_UP]
        && !keyboard_state_array[SDL_SCANCODE_LEFT]
        && !keyboard_state_array[SDL_SCANCODE_RIGHT]
        && !keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        keyboard_state_array[SDL_SCANCODE_W]
        && !keyboard_state_array[SDL_SCANCODE_A]
        && !keyboard_state_array[SDL_SCANCODE_D]
        && !keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardUpLeft(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        keyboard_state_array[SDL_SCANCODE_UP]
        && keyboard_state_array[SDL_SCANCODE_LEFT]
        && !keyboard_state_array[SDL_SCANCODE_RIGHT]
        && !keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        keyboard_state_array[SDL_SCANCODE_W]
        && keyboard_state_array[SDL_SCANCODE_A]
        && !keyboard_state_array[SDL_SCANCODE_D]
        && !keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardUpRight(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        keyboard_state_array[SDL_SCANCODE_UP]
        && !keyboard_state_array[SDL_SCANCODE_LEFT]
        && keyboard_state_array[SDL_SCANCODE_RIGHT]
        && !keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        keyboard_state_array[SDL_SCANCODE_W]
        && !keyboard_state_array[SDL_SCANCODE_A]
        && keyboard_state_array[SDL_SCANCODE_D]
        && !keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardDown(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        !keyboard_state_array[SDL_SCANCODE_UP]
        && !keyboard_state_array[SDL_SCANCODE_LEFT]
        && !keyboard_state_array[SDL_SCANCODE_RIGHT]
        && keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        !keyboard_state_array[SDL_SCANCODE_W]
        && !keyboard_state_array[SDL_SCANCODE_A]
        && !keyboard_state_array[SDL_SCANCODE_D]
        && keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardDownLeft(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        !keyboard_state_array[SDL_SCANCODE_UP]
        && keyboard_state_array[SDL_SCANCODE_LEFT]
        && !keyboard_state_array[SDL_SCANCODE_RIGHT]
        && keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        !keyboard_state_array[SDL_SCANCODE_W]
        && keyboard_state_array[SDL_SCANCODE_A]
        && !keyboard_state_array[SDL_SCANCODE_D]
        && keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardDownRight(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        !keyboard_state_array[SDL_SCANCODE_UP]
        && !keyboard_state_array[SDL_SCANCODE_LEFT]
        && keyboard_state_array[SDL_SCANCODE_RIGHT]
        && keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        !keyboard_state_array[SDL_SCANCODE_W]
        && !keyboard_state_array[SDL_SCANCODE_A]
        && keyboard_state_array[SDL_SCANCODE_D]
        && keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardLeft(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        !keyboard_state_array[SDL_SCANCODE_UP]
        && keyboard_state_array[SDL_SCANCODE_LEFT]
        && !keyboard_state_array[SDL_SCANCODE_RIGHT]
        && !keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        !keyboard_state_array[SDL_SCANCODE_W]
        && keyboard_state_array[SDL_SCANCODE_A]
        && !keyboard_state_array[SDL_SCANCODE_D]
        && !keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardRight(const Uint8 *keyboard_state_array) {
    bool bArrowUp =
        !keyboard_state_array[SDL_SCANCODE_UP]
        && !keyboard_state_array[SDL_SCANCODE_LEFT]
        && keyboard_state_array[SDL_SCANCODE_RIGHT]
        && !keyboard_state_array[SDL_SCANCODE_DOWN];
    bool bWasdUp = 
        !keyboard_state_array[SDL_SCANCODE_W]
        && !keyboard_state_array[SDL_SCANCODE_A]
        && keyboard_state_array[SDL_SCANCODE_D]
        && !keyboard_state_array[SDL_SCANCODE_S];
    return bArrowUp || bWasdUp;
}

bool MainController::isKeyboardF1(const Uint8 *keyboard_state_array) {
    return keyboard_state_array[SDL_SCANCODE_F12];
}

bool MainController::isKeyboardF12(const Uint8 *keyboard_state_array) {
    return keyboard_state_array[SDL_SCANCODE_F12];
}

bool MainController::isKeyboardSpace(const Uint8 *keyboard_state_array) {
    return keyboard_state_array[SDL_SCANCODE_SPACE];
}

void MainController::updatePlayerCoord() {
    const CoordXY &playerCoord = m_pAlientShipState->getPosition();
    std::string sCoordPlayer = "X=" + std::to_string(playerCoord.x())
            + " Y=" + std::to_string(playerCoord.y());
    m_pCoordText->updateText(sCoordPlayer);
}

void MainController::updateFpsValue(int nFps) {
    m_pFpsText->updateText("FPS: ~" + std::to_string(nFps));
    std::cout << "FPS: ~" << nFps << std::endl;
}

GameAlienShipState *MainController::getGameAlienShipState() {
    return m_pAlientShipState;
}

void MainController::generateTanks() {
     m_pRenderWindow->addObject(new RenderTank0(
        new GameTank0State(CoordXY(100,100)),
        m_pTextureTank0,
        1000
    ));

    m_pRenderWindow->addObject(new RenderTank0(
        new GameTank0State(CoordXY(100,200)),
        m_pTextureTank0,
        1000
    ));

    m_pRenderWindow->addObject(new RenderTank0(
        new GameTank0State(CoordXY(200,100)),
        m_pTextureTank0,
        1000
    ));

    m_pRenderWindow->addObject(new RenderTank0(
        new GameTank0State(CoordXY(200,200)),
        m_pTextureTank0,
        1000
    ));
}


