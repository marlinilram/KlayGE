// D3D9RenderFactory.hpp
// KlayGE D3D9渲染引擎抽象工厂 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 改为template实现 (2004.3.4)
//
// 2.0.0
// 初次建立 (2003.8.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDERFACTORY_HPP
#define _D3D9RENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderBuffer.hpp>

#include <d3d9.h>
#include <d3dx9.h>

#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderTexture.hpp>
#include <KlayGE/D3D9/D3D9RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#if defined(DEBUG) | defined(_DEBUG)
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	RenderFactory& D3D9RenderFactoryInstance();

	typedef ConcreteRenderFactory<D3D9RenderEngine, D3D9Texture, D3D9RenderTexture,
			D3D9RenderEffect> D3D9RenderFactoryBase;

	class D3D9RenderFactory : public D3D9RenderFactoryBase
	{
	public:
		D3D9RenderFactory();

		TexturePtr MakeTexture(uint32_t width, uint32_t height, uint16_t numMipMaps,
				PixelFormat format, Texture::TextureUsage usage = Texture::TU_Default);
		RenderTexturePtr MakeRenderTexture(uint32_t width, uint32_t height);
		RenderEffectPtr MakeRenderEffect(std::string const & srcData, uint32_t flags);
			
		VertexStreamPtr MakeVertexStream(VertexStreamType type,
			uint8_t sizeElement, uint8_t numElement, bool staticStream);

		IndexStreamPtr MakeIndexStream(bool staticStream);

		void OnLostDevice();
		void OnResetDevice();

	private:
		std::vector<D3D9TexturePtr> texture_pool_;
		std::vector<D3D9RenderTexturePtr> render_texture_pool_;
		std::vector<D3D9RenderEffectPtr> render_effect_pool_;
		std::vector<D3D9VertexStreamPtr> vertex_stream_pool_;
		std::vector<D3D9IndexStreamPtr> index_stream_pool_;

	private:
		D3D9RenderFactory(D3D9RenderFactory const & rhs);
		D3D9RenderFactory& operator=(D3D9RenderFactory const & rhs);
	};
}

#endif			// _D3D9RENDERFACTORY_HPP
