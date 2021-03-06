#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "Base.h"
#include "frameDetails.h"
#include "IGraphics.h"
#include "Material.h"
#include "renderObject.h"


#include "ShaderModule.h"
#include "TextureModule.h"
#include "ContextModule.h"


namespace de
{
    namespace graphics
    {

        //! Handles all rendering to the screen.
        /*!This class handles all rendering to the screen. To render a Sprite you must first pass the Sprite in with add() and then call render() at the end of the frame.
        */
        class Graphics : public IGraphics
        {
            public:
                //! Initalises the class
                Graphics( const de::graphics::VideoInfo &_videoSettings );
                Graphics();

                virtual ~Graphics();

                //! Renders everything that has been passed into the class since the last time this function was called and then swaps the framebuffer.
                frameDetails render();

                //! Add a 3D mesh for rendering this frame.
                bool add( renderObject *_Object );
				bool add( renderObject *_Object, int _type );

                //!Toggles Fullscreen mode
                void fullscreen( bool _fullscreen );
                //!Resizes screen size.
                void resize( int _screenWidth, int _screenHeight );

				const de::graphics::VideoInfo& getVideoSettings();
				void setVideoSettings( const de::graphics::VideoInfo &_info );

                //!Load the texture with the name "_fileName" in the texture folder.
                Uint32 loadTexture( const std::string& _fileName, int &_width, int &_height );
                //!Unload texture "_texture"
                void unloadTexture( Uint32 &_texture );
                //!Load shader "_name"
                bool loadShader( lua_State* L, Shader &_shader, const std::string &_name );
                //!Unload the shader "_shader"
                bool unloadShader( Shader &_shader );

				void pushSettings( const std::string &_serialisedText );
            private:
                Uint32 loadTexture( const std::string& _fileName, int &_width, int &_height, int _textureDimensions );

                void interalRender();
                frameDetails getFrameInfo();
                de::graphics::VideoInfo getSettings();
                void saveSettings();

            private:

                const SDL_VideoInfo* videoInfo;
                de::graphics::VideoInfo videoSettings;
                lua_State *luaState;

				std::vector< renderObject* > HapticsObjects;
                std::vector< renderObject* > Objects;

                ShaderModule shaderModule;
                TextureModule textureModule;
                ContextModule context;
        };
    }
}

#endif //GRAPHICS_H_
