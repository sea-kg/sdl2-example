#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "game_state.h"
#include "game_rocket_state.h"
#include "game_bioplast_state.h"

class RenderColor {
    public:
        RenderColor(int nR, int nG, int nB, int nA);
        void changeRenderColor(SDL_Renderer* renderer);

    private:
        int m_nR;
        int m_nG;
        int m_nB;
        int m_nA;
};

class RenderObject;

class IRenderWindow {
    public:
        virtual void addObject(RenderObject *pObject) = 0;
        virtual void addRocket(GameRocketState *pRocketState) = 0;
        virtual void addBioplast(GameBioplastState *pState) = 0;
};

class RenderObject {

    public:
        RenderObject(int nPositionZ);
        virtual ~RenderObject();
        int getPositionZ();
        bool isDestroyed();
        void destroy();
        virtual void modify(const GameState& state, IRenderWindow* pRenderWindow);
        virtual void draw(SDL_Renderer* pRenderer) = 0;

    protected:
        bool m_bDestroyed;

    private:
        int m_nPositionZ;
};
