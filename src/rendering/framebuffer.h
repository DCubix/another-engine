#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glad.h"
#include "texture.h"

#include <vector>

namespace ae {
	enum FrameBufferTarget {
		DRFrameBuffer = GL_FRAMEBUFFER,
		DrawFrameBuffer = GL_DRAW_FRAMEBUFFER,
		ReadFrameBuffer = GL_READ_FRAMEBUFFER
	};

	enum Attachment {
		ColorAttachment = GL_COLOR_ATTACHMENT0,
		DepthAttachment = GL_DEPTH_ATTACHMENT,
		StencilAttachment = GL_STENCIL_ATTACHMENT,
		DepthStencilAttachment = GL_DEPTH_STENCIL_ATTACHMENT,
		NoAttachment = GL_NONE
	};

	enum ClearBufferMask {
		ColorBuffer = GL_COLOR_BUFFER_BIT,
		DepthBuffer = GL_DEPTH_BUFFER_BIT,
		StencilBuffer = GL_STENCIL_BUFFER_BIT
	};

	class FrameBuffer {
	public:
		FrameBuffer() = default;
		inline virtual ~FrameBuffer() { free(); }

		void create(uint32 width, uint32 height, uint32 depth = 1);
		void free();

		void color(
			TextureTarget type, TextureFormat format,
			uint32 mip = 0,
			uint32 layer = 0
		);

		void depth();
		void stencil();

		void renderBuffer(
			TextureFormat storage,
			Attachment attachment
		);

		void bind(
			FrameBufferTarget target = FrameBufferTarget::DRFrameBuffer,
			Attachment readBuffer = Attachment::NoAttachment
		);
		void unbind(bool resetViewport = true);

		void drawBuffer(uint32 index);
		void resetDrawBuffers();

		void blit(
			int sx0, int sy0, int sx1, int sy1,
			int dx0, int dy0, int dx1, int dy1,
			ClearBufferMask mask,
			TextureFilter filter
		);

		void clear(ClearBufferMask mask, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

		uint32 id() const { return m_id; }

		uint32 width() const { return m_width; }
		uint32 height() const { return m_height; }
		uint32 depth() const { return m_depth; }

		const std::vector<std::unique_ptr<Texture>>& colorAttachments() { return m_colorAttachments; }
		const std::unique_ptr<Texture>& depthAttachment() { return m_depthAttachment; }
		const std::unique_ptr<Texture>& stencilAttachment() { return m_stencilAttachment; }

	private:
		struct SavedColorAttachment {
			TextureFormat format;
			TextureTarget target;
			uint32 mip;
		};

		uint32 m_id{ 0 }, m_rboID{ 0 };

		uint32 m_width, m_height, m_depth{1};
		int32 m_viewport[4];

		FrameBufferTarget m_bound;

		TextureFormat m_renderBufferStorage;

		std::vector<std::unique_ptr<Texture>> m_colorAttachments;
		std::unique_ptr<Texture> m_depthAttachment, m_stencilAttachment;

		std::vector<SavedColorAttachment> m_savedColorAttachments;
	};
}

#endif // FRAMEBUFFER_H