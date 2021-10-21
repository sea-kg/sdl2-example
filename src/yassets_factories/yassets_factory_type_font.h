#pragma once

#include <yassets_service.h>

class YAssetFactoryTypeFont;

class YAssetFactoryFont : public YAssetFactory {
    public:
        YAssetFactoryFont(
            YAssetsService *pAssetsService,
            YAssetFactoryTypeFont *pFactoryTypeFont,
            const std::wstring &sImagePath,
            const std::vector<std::wstring> &vAlphabets
        );
        virtual YAsset *createAsset() override;
        
    private:
        YAssetFactoryTypeFont *m_pFactoryTypeFont;
        std::wstring m_sImagePath;
        std::vector<std::wstring> m_vAlphabets;
        SDL_Texture *m_pTexture;
};

class YAssetFactoryTypeFont : public YAssetFactoryType {
    public:
        YAssetFactoryTypeFont(YAssetsService *pAssetsService);
        virtual std::wstring getFabricTypeId() override;
        virtual YAssetFactory *createFactory(
            const std::wstring &sAssetFactoryPath,
            const std::wstring &sFactoryId,
            const YJsonObject &jsonFactoryConfig
        ) override;
    private:
        std::wstring TAG;
};
