#include "texture.h"

#include "stb_image.h"

namespace ae {

	namespace intern {
		GLFormat getTextureFormat(TextureFormat format) {
			GLenum ifmt;
			GLint fmt;
			GLenum type;
			uint8 comps;
			switch (format) {
				case TextureFormat::R: ifmt = GL_R8; fmt = GL_RED; type = GL_UNSIGNED_BYTE; comps = 1; break;
				case TextureFormat::RG: ifmt = GL_RG8; fmt = GL_RG; type = GL_UNSIGNED_BYTE; comps = 2; break;
				case TextureFormat::RGB: ifmt = GL_RGB8; fmt = GL_RGB; type = GL_UNSIGNED_BYTE; comps = 3; break;
				case TextureFormat::RGBA: ifmt = GL_RGBA8; fmt = GL_RGBA; type = GL_UNSIGNED_BYTE; comps = 4; break;
				case TextureFormat::Rf: ifmt = GL_R32F; fmt = GL_RED; type = GL_FLOAT; comps = 1; break;
				case TextureFormat::RGf: ifmt = GL_RG32F; fmt = GL_RG; type = GL_FLOAT; comps = 2; break;
				case TextureFormat::RGBf: ifmt = GL_RGB32F; fmt = GL_RGB; type = GL_FLOAT; comps = 3; break;
				case TextureFormat::RGBAf: ifmt = GL_RGBA32F; fmt = GL_RGBA; type = GL_FLOAT; comps = 4; break;
				case TextureFormat::Depthf: ifmt = GL_DEPTH_COMPONENT24; fmt = GL_DEPTH_COMPONENT; type = GL_FLOAT; comps = 1; break;
				case TextureFormat::DepthStencil: ifmt = GL_DEPTH24_STENCIL8; fmt = GL_DEPTH_STENCIL; type = GL_FLOAT; comps = 2; break;
			}
			return std::make_tuple(ifmt, fmt, type, comps);
		}
	}
	
	Texture::Texture() {
		create();
	}

	Texture::~Texture() {
		free();
	}

	void Texture::create() {
		glGenTextures(1, &m_id);
	}

	void Texture::free() {
		if (m_id) glDeleteTextures(1, &m_id);
	}

	void Texture::bind(uint32 slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GLenum(m_target), m_id);
	}

	void Texture::unbind() {
		glBindTexture(GLenum(m_target), 0);
	}

	void Texture::setSize(uint32 width, uint32 height, uint32 depth) {
		m_width = width;
		m_height = height;
		m_depth = depth;
	}

	void Texture::setData(const void* data, TextureFormat format) {
		glBindTexture(GLenum(m_target), m_id);
		auto [ifmt, fmt, type, comps] = intern::getTextureFormat(format);
		switch (m_target) {
			default: break;
			case TextureTarget::Texture1D: glTexImage1D(GLenum(m_target), 0, ifmt, m_width, 0, fmt, type, data); break;
			case TextureTarget::Texture2D: glTexImage2D(GLenum(m_target), 0, ifmt, m_width, m_height, 0, fmt, type, data); break;
			case TextureTarget::Texture3D: glTexImage3D(GLenum(m_target), 0, ifmt, m_width, m_height, m_depth, 0, fmt, type, data); break;
		}
		
		if (m_mag == TextureFilter::LinearMipLinear ||
			m_mag == TextureFilter::LinearMipNearest ||
			m_mag == TextureFilter::NearestMipLinear ||
			m_mag == TextureFilter::NearestMipNearest)
		{
			glGenerateMipmap(GLenum(m_target));
		}
	}

	void Texture::update(const void* data, TextureFormat format) {
		glBindTexture(GLenum(m_target), m_id);
		auto [ifmt, fmt, type, comps] = intern::getTextureFormat(format);
		switch (m_target) {
			default: break;
			case TextureTarget::Texture1D: glTexSubImage1D(GLenum(m_target), 0, 0, m_width, fmt, type, data); break;
			case TextureTarget::Texture2D: glTexSubImage2D(GLenum(m_target), 0, 0, 0, m_width, m_height, fmt, type, data); break;
			case TextureTarget::Texture3D: glTexSubImage3D(GLenum(m_target), 0, 0, 0, 0, m_width, m_height, m_depth, fmt, type, data); break;
		}
		if (m_mag == TextureFilter::LinearMipLinear ||
			m_mag == TextureFilter::LinearMipNearest ||
			m_mag == TextureFilter::NearestMipLinear ||
			m_mag == TextureFilter::NearestMipNearest)
		{
			glGenerateMipmap(GLenum(m_target));
		}
	}

	void Texture::setCubeMapData(const void* data, TextureFormat format, CubeMapSide side) {
		glBindTexture(GLenum(m_target), m_id);
		auto [ifmt, fmt, type, comps] = intern::getTextureFormat(format);
		glTexImage2D(GLenum(side), 0, ifmt, m_width, m_height, 0, fmt, type, data);
		if (m_mag == TextureFilter::LinearMipLinear ||
			m_mag == TextureFilter::LinearMipNearest ||
			m_mag == TextureFilter::NearestMipLinear ||
			m_mag == TextureFilter::NearestMipNearest)
		{
			glGenerateMipmap(GLenum(m_target));
		}
	}

	void Texture::target(TextureTarget target) {
		m_target = target;
	}

	void Texture::wrap(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR) {
		glTexParameteri(GLenum(m_target), GL_TEXTURE_WRAP_S, GLenum(wrapS));
		glTexParameteri(GLenum(m_target), GL_TEXTURE_WRAP_T, GLenum(wrapT));
		if (m_target == TextureTarget::CubeMap || m_target == TextureTarget::Texture3D)
			glTexParameteri(GLenum(m_target), GL_TEXTURE_WRAP_R, GLenum(wrapR));
	}

	void Texture::filter(TextureFilter min, TextureFilter mag) {
		glTexParameteri(GLenum(m_target), GL_TEXTURE_MIN_FILTER, GLenum(min));
		glTexParameteri(GLenum(m_target), GL_TEXTURE_MAG_FILTER, GLenum(mag));
	}

	TextureFactory::TextureFactory(const std::string& fileName) {
		this->fileName(fileName);
	}

	ResourcePtr TextureFactory::load() {
		if (!m_ptr) {
			m_ptr = std::make_shared<Texture>();
			auto file = FileSystem::ston().open(fileName());
			auto sz = file.size();
			std::vector<uint8> data;
			data.resize(sz);

			if (file.read(data.data(), sz) == sz) {
				int w, h, comp;
				stbi_set_flip_vertically_on_load(1);
				unsigned char* imgData = stbi_load_from_memory(data.data(), sz, &w, &h, &comp, STBI_rgb_alpha);
				if (imgData) {
					m_ptr->setSize(w, h);
					m_ptr->bind();
					m_ptr->filter(TextureFilter::Linear, TextureFilter::LinearMipLinear);
					m_ptr->wrap(TextureWrap::Repeat, TextureWrap::Repeat);
					m_ptr->setData(imgData, TextureFormat::RGBA);
					m_ptr->unbind();
					stbi_image_free(imgData);
				}
			}
			file.close();
		}
		return m_ptr;
	}

}