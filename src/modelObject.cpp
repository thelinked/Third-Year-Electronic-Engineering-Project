#include "modelObject.h"
#include "GameServices.h"
#include "openGL.h"

modelObject::modelObject() : refresh(false), usingTexture(false), writingToDepth(false), depthTest(false), alphaTest(false), blending(false), active(false), type(GL_TRIANGLES)
{
	bindTexture.push_back( GL_TEXTURE0 );
	bindTexture.push_back( GL_TEXTURE1 );
	bindTexture.push_back( GL_TEXTURE2 );
	bindTexture.push_back( GL_TEXTURE3 );
	bindTexture.push_back( GL_TEXTURE4 );
	bindTexture.push_back( GL_TEXTURE5 );
}
modelObject::~modelObject()
{

}

modelObject& modelObject::writeToDepth( bool _writeToDepth )
{
	writingToDepth = _writeToDepth;
	return *this;
}
modelObject& modelObject::depth( bool _depth )
{
	depthTest = _depth;
	return *this;
}
modelObject& modelObject::alpha( bool _alpha )
{
	alphaTest = _alpha;
	return *this;
}
modelObject& modelObject::blend( bool _blend )
{
	blending = _blend;
	return *this;
}
modelObject& modelObject::ccw( bool _ccw )
{
	frontFaceCCW = _ccw;
	return *this;
}

modelObject& modelObject::setType( const std::string &_type )
{
    if( _type == "TriangleStrip" )
        type = GL_TRIANGLE_STRIP;
    else if( _type == "Triangles" )
        type = GL_TRIANGLES;
    else if( _type == "Point" )
        type = GL_POINT;
    else if( _type == "Points" )
        type = GL_POINTS;
    else if( _type == "Line" )
        type = GL_LINE;
    else if( _type == "Lines" )
        type = GL_LINES;

	return *this;
}

modelObject& modelObject::setTexture( const std::string &_texture, const std::string &_value, int texNumber )
{
	shader.setUniform( _texture, texNumber );
	textures[_texture] = texPair( _value, de::Engine::Resources().getTexture(_value) );

	usingTexture = true;

	return *this;
}
modelObject& modelObject::load( const std::string &_mesh, const std::string &_shader )
{
	meshName = _mesh;
	shaderName = _shader;

	return loadMesh();
}
modelObject& modelObject::loadMesh()
{
	vbo = de::Engine::Resources().getMesh( meshName );
    shader = de::Engine::Resources().getShader( shaderName );
    AttributeNames = shader.getAttributeNames();
	iterNames = AttributeNames.begin();

	//de::io::log << "Textures: " << vbo.textures.size() << "\n";
	
	for( int i = 0; i!=vbo.textures.size();i++ )
	{
		//de::io::log << "Model " << meshName << " contains texture: " << vbo.textures[i] << "\n";
	}
	/*
	if(  vbo.textures.size() )
	{
		de::io::log << "Model " << meshName << " contains texture: " << vbo.textures[0] << "\n";
	}
	*/
	active = true;
	return *this;
}

void modelObject::reload()
{
	refresh = true;

}
void modelObject::render()
{
	de::Engine::Graphics().add( this );
}

void modelObject::actualRender()
{
	if( refresh)
	{
		refresh = false;
		loadMesh();
	}

	if( active )
	{

		if( usingTexture )
		{
			CHECKGL( glEnable( GL_TEXTURE_2D ) );

			std::map<std::string, texPair>::iterator textureIter = textures.begin(), end = textures.end();
			std::vector<int>::iterator bindIter = bindTexture.begin(), bindEnd = bindTexture.end();

			for( textureIter; textureIter != end && bindIter!=bindEnd; ++textureIter, ++bindIter )
			{
				CHECKGL( glActiveTexture( (*bindIter) ) );
				CHECKGL( glBindTexture( GL_TEXTURE_2D, textureIter->second.texture ) );

				//de::io::tests << "Binding texture: " << textureIter->second.textureName << " : " << textureIter->first << " : " << textureIter->second.texture << "\n";
			}			
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

		if( frontFaceCCW ) 
		{
			CHECKGL( glFrontFace(GL_CCW) );
		}
		else CHECKGL(glFrontFace(GL_CW) );

						

		shader.bindShader();
		CHECKGL( glBindBuffer( GL_ARRAY_BUFFER, vbo.vertexBuffer ) );
		
		for( iterNames = AttributeNames.begin(); iterNames != AttributeNames.end(); ++iterNames )
		{
			if( vbo.Attribs.find( (*iterNames) ) != vbo.Attribs.end() )
			{
				CHECKGL( glVertexAttribPointer(
					shader.getAttribute( (*iterNames) ),			// attribute
					vbo.Attribs[(*iterNames)].size,					// size
					GL_FLOAT,										// type
					GL_FALSE,			// normalized?
					vbo.Attribs[(*iterNames)].stride,				// stride
					(void*)(vbo.Attribs[(*iterNames)].offset)		// array buffer offset
				) );
				CHECKGL( glEnableVertexAttribArray( shader.getAttribute( (*iterNames) ) ) );
			}
			else
			{
				de::io::error << "Attribute: " << (*iterNames) << " missing:\n";
				de::io::error << "Available Attributes are ";

				int i = 0;
				std::map< std::string, de::graphics::AttInfo >::iterator iterAtt =  vbo.Attribs.begin();
				for(iterAtt; iterAtt!=vbo.Attribs.end();++iterAtt,++i)
				{
					de::io::error << i+1 << ": " << iterAtt->first << " \n";
				}
			}
		}
		
		CHECKGL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo.elementBuffer ) );
			CHECKGL( glDrawElements(
				type,				// mode
				vbo.triangles,		// count
				GL_UNSIGNED_INT,    // type
				(void*)0            // element array buffer offset
			) );

			
		for( iterNames = AttributeNames.begin(); iterNames != AttributeNames.end(); ++iterNames )
		{
			if( vbo.Attribs.find( *iterNames ) != vbo.Attribs.end() )
			{
				CHECKGL( glDisableVertexAttribArray( shader.getAttribute( (*iterNames) ) ) );
			}
		}

		CHECKGL( glUseProgram( 0 ) );
	}
}
