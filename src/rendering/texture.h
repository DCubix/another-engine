#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad.h"
#include "resource_manager.h"
#include "integer.hpp"

#include <tuple>
#include <vector>

namespace ae {

	enum class TextureTarget : GLenum {
		Texture1D = GL_TEXTURE_1D,
		Texture2D = GL_TEXTURE_2D,
		Texture3D = GL_TEXTURE_3D,
		CubeMap = GL_TEXTURE_CUBE_MAP
	};

	enum class CubeMapSide : GLenum {
		CubeMapPX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		CubeMapNX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		CubeMapPY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		CubeMapNY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		CubeMapPZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		CubeMapNZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	enum class TextureWrap : GLenum {
		None = 0,
		Repeat = GL_REPEAT,
		Clamp = GL_CLAMP_TO_EDGE
	};

	enum class TextureFilter : GLenum {
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR,
		NearestMipNearest = GL_NEAREST_MIPMAP_NEAREST,
		NearestMipLinear = GL_NEAREST_MIPMAP_LINEAR,
		LinearMipLinear = GL_LINEAR_MIPMAP_LINEAR,
		LinearMipNearest = GL_LINEAR_MIPMAP_NEAREST
	};

	enum class TextureFormat : uint8 {
		R = 0,
		RG,
		RGB,
		RGBA,
		Rf,
		RGf,
		RGBf,
		RGBAf,
		Depthf,
		DepthStencil
	};

	namespace intern {
		using GLFormat = std::tuple<GLint, GLenum, GLenum, uint8>;
		GLFormat getTextureFormat(TextureFormat format);
	}

	class Texture : public ResourceBase {
	public:
		Texture();
		~Texture();

		void create();
		void free();

		void bind(uint32 slot = 0);
		void unbind();

		GLuint id() const { return m_id; }
		uint32 width() const { return m_width; }
		uint32 height() const { return m_height; }
		uint32 depth() const { return m_depth; }

		TextureTarget target() const { return m_target; }

		void target(TextureTarget target);
		void wrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR = TextureWrap::Repeat);
		void filter(TextureFilter min, TextureFilter mag);

		void setSize(uint32 width, uint32 height = 0, uint32 depth = 0);
		void setData(const void* data, TextureFormat format);
		void update(const void* data, TextureFormat format);
		void setCubeMapData(const void* data, TextureFormat format, CubeMapSide side);

	private:
		GLuint m_id{ 0 };
		TextureTarget m_target{ TextureTarget::Texture2D };
		TextureFilter m_mag{ TextureFilter::Linear };

		uint32 m_width{ 0 }, m_height{ 0 }, m_depth{ 0 };
	};

	using TexturePtr = std::shared_ptr<Texture>;
	class TextureFactory : public ResourceFactory {
	public:
		explicit TextureFactory(const std::string& fileName);
		virtual ResourcePtr load() override;
	private:
		TexturePtr m_ptr;
	};

}

#endif // TEXTURE_H
