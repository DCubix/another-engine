#include "framebuffer.h"

#include <iostream>

namespace ae {
	void FrameBuffer::destroy() {
		if (m_id) {
			glDeleteFramebuffers(1, &m_id);
			m_id = 0;
		}
		if (m_rboID) {
			glDeleteRenderbuffers(1, &m_rboID);
			m_rboID = 0;
		}
		m_colorAttachments.clear();
	}

	void FrameBuffer::create(uint32 width, uint32 height, uint32 depth) {
		m_width = width;
		m_height = height;
		m_depth = depth;

		glGenFramebuffers(1, &m_id);
	}

	void FrameBuffer::color(TextureTarget type, TextureFormat format, uint32 mip, uint32 layer) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);

		std::unique_ptr<Texture> tex = std::make_unique<Texture>();
		tex->setSize(m_width, m_height);
		tex->bind();
		tex->wrap(TextureWrap::Clamp, TextureWrap::Clamp);
		tex->filter(TextureFilter::LinearMipLinear, TextureFilter::Linear);
		tex->setData(nullptr, format);

		if (type == TextureTarget::CubeMap) {
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapNX);
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapNY);
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapNZ);
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapPX);
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapPY);
			tex->setCubeMapData(nullptr, format, CubeMapSide::CubeMapPZ);
		} else {
			tex->setData(nullptr, format);
		}

		SavedColorAttachment sca;
		sca.format = format;
		sca.target = type;
		sca.mip = mip;
		m_savedColorAttachments.push_back(sca);

		std::vector<GLenum> db;
		uint32 att = m_colorAttachments.size();
		for (uint32 i = 0; i < att + 1; i++) {
			db.push_back(GL_COLOR_ATTACHMENT0 + i);
		}

		GLenum atc = GL_COLOR_ATTACHMENT0 + att;
		switch (type) {
			case TextureTarget::Texture1D:
				glFramebufferTexture1D(GL_FRAMEBUFFER, atc, GLenum(type), tex->id(), mip);
				break;
			case TextureTarget::Texture2D:
				glFramebufferTexture2D(GL_FRAMEBUFFER, atc, GLenum(type), tex->id(), mip);
				break;
			case TextureTarget::Texture3D:
				glFramebufferTexture3D(GL_FRAMEBUFFER, atc, GLenum(type), tex->id(), mip, layer);
				break;
			default:
				glFramebufferTexture(GL_FRAMEBUFFER,  atc, tex->id(), mip);
				break;
		}

		glDrawBuffers(db.size(), db.data());

		m_colorAttachments.push_back(std::move(tex));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::depth(uint32 depthSize) {
		if (m_depthAttachment->id() != 0) {
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_id);

		std::unique_ptr<Texture> tex = std::make_unique<Texture>();
		tex->setSize(m_width, m_height);
		tex->bind();
		tex->wrap(TextureWrap::Clamp, TextureWrap::Clamp);
		tex->filter(TextureFilter::Nearest, TextureFilter::Nearest);
		tex->setData(nullptr, TextureFormat::Depthf);

		glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_2D,
				tex->id(),
				0
		);

		m_depthAttachment = std::move(tex);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::stencil() {
		if (m_stencilAttachment->id() != 0) {
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_id);

		std::unique_ptr<Texture> tex = std::make_unique<Texture>();
		tex->setSize(m_width, m_height);
		tex->bind();
		tex->wrap(TextureWrap::Clamp, TextureWrap::Clamp);
		tex->filter(TextureFilter::Nearest, TextureFilter::Nearest);
		tex->setData(nullptr, TextureFormat::R);

		glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT,
				GL_TEXTURE_2D,
				tex->id(),
				0
		);

		m_stencilAttachment = std::move(tex);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::renderBuffer(
		TextureFormat storage,
		Attachment attachment
	) {
		if (m_rboID != 0) {
			return;
		}

		m_renderBufferStorage = storage;
		auto fmt = intern::getTextureFormat(storage);

		glGenRenderbuffers(1, &m_rboID);

		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rboID);
		glRenderbufferStorage(GL_RENDERBUFFER, std::get<0>(fmt), m_width, m_height);
		glFramebufferRenderbuffer(
				GL_FRAMEBUFFER,
				attachment,
				GL_RENDERBUFFER,
				m_rboID
		);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			glDeleteRenderbuffers(1, &m_rboID);
			m_rboID = 0;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::drawBuffer(uint32 index) {
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + index);
	}

	void FrameBuffer::resetDrawBuffers() {
		std::vector<GLenum> db;
		int32 att = m_colorAttachments.size();
		for (int i = 0; i < att; i++) {
			db.push_back(GL_COLOR_ATTACHMENT0 + i);
		}
		glDrawBuffers(db.size(), db.data());
	}

	void FrameBuffer::blit(
		int sx0, int sy0, int sx1, int sy1,
		int dx0, int dy0, int dx1, int dy1,
		ClearBufferMask mask,
		TextureFilter filter)
	{
		glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, GLenum(filter));
	}

	void FrameBuffer::bind(FrameBufferTarget target, Attachment readBuffer) {
		m_bound = target;
		glGetIntegerv(GL_VIEWPORT, m_viewport);
		glBindFramebuffer(target, m_id);
		glViewport(0, 0, m_width, m_height);
		if (target == FrameBufferTarget::ReadFrameBuffer) {
			glReadBuffer(readBuffer);
		}
	}

	void FrameBuffer::unbind(bool resetViewport) {
		glBindFramebuffer(m_bound, 0);
		if (resetViewport) {
			glViewport(
				m_viewport[0],
				m_viewport[1],
				m_viewport[2],
				m_viewport[3]
			);
		}
	}
}