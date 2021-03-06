#ifndef MODELOBJECT_H
#define MODELOBJECT_H

#include "Base.h"
#include "renderObject.h"

#include "VBO.h"
#include "Material.h"

class hapticsOpject;

struct texPair
{
	std::string textureName;
	int texture;
	texPair() {}
	~texPair() {}
	texPair( const std::string &_textureName, int _texture ) 
		: textureName(_textureName), texture(_texture) {}
};

class modelObject : public de::graphics::renderObject
{
    public:
		friend class hapticsOpject;

        /** Default constructor */
        modelObject();
        /** Default destructor */
        virtual ~modelObject();

        template<typename T>
        modelObject& setUniform( const std::string &_name, const T &_value )
        {
            shader.setUniform( _name, _value );
            return *this;
        }
        modelObject& writeToDepth( bool _writeToDepth );
		modelObject& depth( bool _depth );
		modelObject& alpha( bool _alpha );
		modelObject& blend( bool _blend );
		modelObject& ccw( bool _ccw );

        modelObject& setType( const std::string &_type );
        modelObject& load( const std::string &_mesh, const std::string &_shader );
		modelObject& setTexture( const std::string &_texture, const std::string &_value, int texNumber );

        void reload();
        void render();
		void actualRender();

		bool refresh;
    private:
		
		modelObject& loadMesh();

        bool usingTexture, writingToDepth, depthTest, alphaTest, blending, active, frontFaceCCW;
        int type;
		std::string shaderName, meshName;


		std::map<std::string, texPair> textures;
		std::vector<int> bindTexture;

		de::graphics::VBO vbo;
        de::graphics::Shader shader;
		std::vector<std::string> AttributeNames;
		std::vector<std::string>::iterator iterNames;
        
};


#endif //MODELOBJECT_H