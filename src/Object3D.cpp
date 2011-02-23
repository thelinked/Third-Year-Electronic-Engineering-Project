#include "Object3D.h"
#include "GameServices.h"
#include "openGL.h"
#include <aiTypes.h>
#include <aiScene.h> 
#include <aiPostProcess.h>

namespace
{
    static Uint32 make_buffer( GLenum target, const void *buffer_data, GLsizei buffer_size )
    {
        Uint32 buffer;
        CHECKGL( glGenBuffers( 1, &buffer ) );
        CHECKGL( glBindBuffer( target, buffer ) );
        CHECKGL( glBufferData( target, buffer_size, buffer_data, GL_DYNAMIC_DRAW ) );
        return buffer;
    }
}



Object3D::Object3D()
    : refreshVBO(false), usingTexture(false), writingToDepth(true), alphaTest(true), blending(true), type( GL_TRIANGLES )
{
}

Object3D::~Object3D()
{
	clear();
}

void Object3D::clear()
{
	CHECKGL( glDeleteBuffers( 1, &vertexBuffer ) );
	CHECKGL( glDeleteBuffers( 1, &elementBuffer ) );

	VertexBuffer.clear();
	ElementBuffer.clear();
	AttributeInfo.clear();
	AttributeElemenRange.clear();
	AttributeNames.clear();

	refreshVBO = true;
}

Object3D& Object3D::set( const std::string &_shaderName )
{
	AttributeNames.clear();
	AttributeInfo.clear();
	AttributeElemenRange.clear();

	AttIter = AttributeInfo.begin();

    shaderName = _shaderName;
    shader = de::Engine::Resources().getShader( shaderName );
    AttributeNames = shader.getAttributeNames();

    std::vector<std::string>::iterator start, end = AttributeNames.end();
    for( start = AttributeNames.begin(); start != end; ++start )
    {
        AttributeInfo[(*start)] = bufferInfo();
        AttributeElemenRange[(*start)] = bufferInfo();
    }

    return *this;
}

Object3D& Object3D::setTexture( const std::string &_name, const std::string &_value )
{
    textureName = _value;
    //shader.setUniform( _name, Engine::Resources().getTexture(_value) );
	shader.setUniform( _name, 0 );
    usingTexture = true;
    texture = de::Engine::Resources().getTexture(_value);

    return *this;
}

Object3D& Object3D::writeToDepth( bool _writeToDepth )
{
    writingToDepth = _writeToDepth;
    return *this;
}
Object3D& Object3D::depth( bool _depth )
{
	depthTest = _depth;
	return *this;
}
Object3D& Object3D::alpha( bool _alpha )
{
	alphaTest = _alpha;
	return *this;
}
Object3D& Object3D::blend( bool _blend )
{
	blending = _blend;
	return *this;
}




Object3D& Object3D::add( const std::string &_name, const std::vector<de::graphics::vertex> &_vertexBuffer, const std::vector<int> &_elementBuffer )
{
	ElementBuffer.clear();
    ElementBuffer = _elementBuffer;
    add( _name, _vertexBuffer );

    return *this;
}

Object3D& Object3D::add( const std::string &_name, const std::vector<de::graphics::vertex> &_vertexBuffer )
{
    std::map<std::string, bufferInfo>::iterator find = AttributeInfo.find( _name );
    if( find == AttributeInfo.end() || !find->second.isSet )
    {
        de::graphics::AttributeTypes type;
        shader.getAttribute( _name, type );
        AttributeInfo[_name] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), _vertexBuffer.size(), type );

        for( std::vector<de::graphics::vertex>::const_iterator vec4Iter = _vertexBuffer.begin(); vec4Iter != _vertexBuffer.end(); ++vec4Iter )
        {
            VertexBuffer.push_back( (*vec4Iter) );
        }
        refreshVBO = true;
    }
    else
    {
        if( find->second.size == _vertexBuffer.size() )
        {
            int i(0);
            for( std::vector<de::graphics::vertex>::const_iterator vec4Iter = _vertexBuffer.begin(); vec4Iter != _vertexBuffer.end(); ++vec4Iter, ++i )
            {
                VertexBuffer[find->second.size + i] = (*vec4Iter);
            }
            refreshVBO = true;
        }
    }
    return *this;
}

Object3D& Object3D::setType( const std::string &_type )
{
    if( _type == "TriangleStrip" )
        type = GL_TRIANGLE_STRIP;
    else if( _type == "Triangles" )
        type = GL_TRIANGLES;
    else if( _type == "Point" )
        type = GL_POINT;
    else if( _type == "Points" )
        type = GL_POINTS;

    return *this;
}


Object3D& Object3D::load( const std::string &_name, const std::string &_shader )
{
    return loadCTM( _name, _shader );
	//return loadAssimp( _name, _shader );
}

Object3D& Object3D::loadAssimp( const std::string &_name, const std::string &_shader )
{
    using namespace de::filesystem;
    using namespace de::graphics;

	set( _shader );
    type = GL_TRIANGLES;
    refreshVBO = true;
    meshName = _name;

    std::string path( Roots->get( root::MESHES )+meshName+".lwo" );
    const aiScene* scene = importer.ReadFile( path.c_str(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_MakeLeftHanded |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeMeshes );

    if( !scene )
    {
        de::io::error << importer.GetErrorString() << "\n";
    }

	if( scene->HasMeshes() )
	{
		de::io::log << "It has " << scene->mNumMeshes << " meshes!" << "\n";


		aiMesh* _mesh = scene->mMeshes[0];

		
		if( _mesh->HasPositions() )
		{
			de::io::log << "Mesh has " <<  _mesh->mNumVertices << " Verts and contains ";
			if( _mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE )
			{
				de::io::log << "Triangles\n";
			}
			
			de::graphics::AttributeTypes attType;
			shader.getAttribute( "Position", attType );
			AttributeInfo["Position"] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), _mesh->mNumVertices, attType );

			int i, num = _mesh->mNumVertices;
			aiVector3D* verts = _mesh->mVertices;
			for( i = 0; i!=num; ++i)
			{
				VertexBuffer.push_back( vertex( verts[num].x, verts[num].y, verts[num].y, 1.0f ) );
			}
			de::io::log << "VertexBuffer.size() = " << VertexBuffer.size() << "\n";

			
			num = _mesh->mNumFaces;
			aiFace* faces = _mesh->mFaces;
			for( i = 0; i!=num; ++i)
			{
				ElementBuffer.push_back( faces[i].mIndices[0] );
				ElementBuffer.push_back( faces[i].mIndices[1] );
				ElementBuffer.push_back( faces[i].mIndices[2] );
			}
			de::io::log << "ElementBuffer.size() = " << ElementBuffer.size() << "\n";
		}

		if( _mesh->HasNormals() )
		{
			de::io::log << "Mesh has normals\n";
			
			
			de::graphics::AttributeTypes attTypeNormal;
			shader.getAttribute( "Normal", attTypeNormal );
			AttributeInfo["Normal"] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), _mesh->mNumVertices, attTypeNormal );

			int i, num = _mesh->mNumVertices;
			aiVector3D* verts = _mesh->mNormals;
			for( i = 0; i!=num; ++i)
			{
				VertexBuffer.push_back( vertex( verts[num].x, verts[num].y, verts[num].y, 1.0f ) );
			}
		}


		de::io::log << "Mesh has " << _mesh->GetNumUVChannels() << " UVs.\n";



	}
    return *this;
}

Object3D& Object3D::loadCTM( const std::string &_name, const std::string &_shader )
{
    using namespace de::filesystem;
    using namespace de::graphics;

    set( _shader );
    type = GL_TRIANGLES;
    refreshVBO = true;
    meshName = _name;

    CTMcontext ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, (Roots->get( root::MESHES )+meshName+".ctm").c_str() );
    unsigned int indexCount = 3 * ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);
    unsigned int vertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);


    int i;
    const CTMuint* indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    for( i = 0; i!=indexCount; ++i)
    {
        ElementBuffer.push_back( indices[i] );
    }


    const CTMfloat* vertices = ctmGetFloatArray(ctmContext, CTM_VERTICES);
    de::graphics::AttributeTypes attType;
    shader.getAttribute( "Position", attType );
    AttributeInfo["Position"] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), vertexCount, attType );

    for( i = 0; i!=vertexCount; ++i)
    {
        int pos = i*3;
        VertexBuffer.push_back( vertex( vertices[pos], vertices[pos+1], vertices[pos+2], 1.0f ) );
    }


    if( ctmGetInteger(ctmContext, CTM_HAS_NORMALS) )
    {
        const CTMfloat* normals = ctmGetFloatArray(ctmContext, CTM_NORMALS);
        de::graphics::AttributeTypes attTypeNormal;
        shader.getAttribute( "Normal", attTypeNormal );
        AttributeInfo["Normal"] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), vertexCount, attTypeNormal );


        for( i = 0; i!=vertexCount; ++i)
        {
            int pos = i*3;
            VertexBuffer.push_back( vertex( normals[pos], normals[pos+1], normals[pos+2], 0.0f ) );
        }
    }
    else
    {
        de::io::error << "No normals found in mesh:" << meshName << "\n";
    }

    const CTMfloat* uv = ctmGetFloatArray(ctmContext, CTM_UV_MAP_1);
    if( uv )
    {
        std::string texture = stripFileEnding( ctmGetUVMapString( ctmContext, CTM_UV_MAP_1, CTM_FILE_NAME ) );
        setUniform( "Texture0", texture );
        de::io::log << "texture = " << texture << "\n";

        de::graphics::AttributeTypes uvType;
        shader.getAttribute( "Tex", uvType );
        AttributeInfo["Tex"] = bufferInfo( VertexBuffer.size()*sizeof(de::graphics::vertex), vertexCount, uvType );
        for( i = 0; i!=vertexCount; ++i)
        {
            int pos = i*2;
            VertexBuffer.push_back( vertex( uv[pos], uv[pos+1], 0.0f, 0.0f ) );
        }
    }

    ctmFreeContext(ctmContext);

    return *this;
}

void Object3D::makeBuffers()
{
    vertexBuffer = make_buffer( GL_ARRAY_BUFFER, &(VertexBuffer[0]), VertexBuffer.size()*sizeof(de::graphics::vertex) );
    elementBuffer = make_buffer( GL_ELEMENT_ARRAY_BUFFER, &(ElementBuffer[0]), ElementBuffer.size()*sizeof(int) );
    refreshVBO = false;
}

void Object3D::reload()
{
    refreshVBO = true;
    shader.setUniform( "Texture0", de::Engine::Resources().getTexture(textureName) );

    texture = de::Engine::Resources().getTexture(textureName);
    shader = de::Engine::Resources().getShader( shaderName );
}

void Object3D::render()
{
    de::Engine::Graphics().add( this );
}

void Object3D::localRender()
{
    if( refreshVBO )
    {
        makeBuffers();
    }

    if( usingTexture )
    {
        CHECKGL( glEnable( GL_TEXTURE_2D ) );
        CHECKGL( glActiveTexture( GL_TEXTURE0 ) );
        CHECKGL( glBindTexture( GL_TEXTURE_2D, texture ) );
    }
    else
        CHECKGL( glDisable( GL_TEXTURE_2D ) );

    if( writingToDepth )
	{
		CHECKGL( glDepthMask( GL_TRUE ) );
	}
    else glDepthMask( GL_FALSE );

	if( depthTest ) 
	{
		CHECKGL( glEnable( GL_DEPTH_TEST ) );
	}
    else CHECKGL( glDisable( GL_DEPTH_TEST ) );

	if( alphaTest ) 
	{
		CHECKGL( glEnable( GL_ALPHA_TEST ) );
	}
    else CHECKGL( glDisable( GL_ALPHA_TEST ) );

	if( blending ) 
	{
		CHECKGL( glEnable( GL_BLEND ) );
	}
    else CHECKGL( glDisable( GL_BLEND ) );


    shader.bindShader();
    CHECKGL( glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer ) );
    for( AttIter = AttributeInfo.begin(); AttIter != AttributeInfo.end(); ++AttIter )
    {
        if( AttIter->second.isSet )
        {
            int size(AttIter->second.type);

            CHECKGL( glVertexAttribPointer(
                shader.getAttribute( AttIter->first ),        // attribute
                size,                                         // size
                GL_FLOAT,                                     // type
                GL_FALSE,                                     // normalized?
                sizeof(de::graphics::vertex),                 // stride
                (void*)(AttributeInfo[AttIter->first].offset) // array buffer offset
            ) );
            CHECKGL( glEnableVertexAttribArray( shader.getAttribute( AttIter->first) ) );
        }
        else if( AttIter->second.warning )
        {
            AttIter->second.warning = false;
            de::io::error << "Attribute missing:\"" << AttIter->first << "\". Using Shader:\"" << shaderName << "\" Mesh:\"" << meshName << "\"\n";
        }
    }

    CHECKGL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementBuffer ) );
        CHECKGL( glDrawElements(
            type,  // mode
            ElementBuffer.size(),                  // count
            GL_UNSIGNED_INT,    // type
            (void*)0            // element array buffer offset
        ) );

    for( AttIter = AttributeInfo.begin(); AttIter != AttributeInfo.end(); ++AttIter )
    {
        CHECKGL( glDisableVertexAttribArray( shader.getAttribute( AttIter->first ) ) );
    }
    CHECKGL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
    CHECKGL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
    CHECKGL( glUseProgram( 0 ) );
}

