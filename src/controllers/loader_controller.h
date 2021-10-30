#pragma once
#include <string>
#include "render_ui.h"
#include <yassets_ui.h>

class LoaderController {
    public:
        LoaderController(
            const std::wstring &sResourceDir,
            RenderWindow *pRenderWindow,
            GameState *pGameState
        );
        void init();
        void deinit();
        void updateText(const std::wstring &sNewText);
        void setProgressMax(int nVal);
        void setProgressCurrent(int nVal);
        void addToProgressMax(int nVal);
        void addToProgressCurrent(int nVal);
        void draw();

    private:
        void addObject(RenderObject *pObject);
        std::wstring m_sResourceDir;
        GameState *m_pGameState;

        RenderWindow *m_pRenderWindow;
        SDL_Texture *m_pTextureLogoBig;
        std::vector<RenderObject *> m_vObjects;

        YAssetText *m_pAssetText;
        YAssetProgressBar *m_pAssetProgressBar;
        YAssetBackground *m_pAssetBackground;
        int m_nProgressCurrent;
        int m_nProgressMax;
};