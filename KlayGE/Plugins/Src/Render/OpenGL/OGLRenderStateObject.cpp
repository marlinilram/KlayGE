// OGLRenderStateObject.cpp
// KlayGE OpenGL渲染状态对象类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2008-2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 支持Depth Clamp (2009.8.5)
//
// 3.7.0
// 初次建立 (2007.7.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>

namespace KlayGE
{
	OGLRenderStateObject::OGLRenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,
			BlendStateDesc const & bs_desc)
		: RenderStateObject(rs_desc, dss_desc, bs_desc),
			ogl_polygon_mode_(OGLMapping::Mapping(rs_desc_.polygon_mode)),
			ogl_shade_mode_(OGLMapping::Mapping(rs_desc_.shade_mode)),
			ogl_front_face_(rs_desc_.front_face_ccw ? GL_CCW : GL_CW),
			ogl_depth_write_mask_(dss_desc_.depth_write_mask ? GL_TRUE : GL_FALSE),
			ogl_depth_func_(OGLMapping::Mapping(dss_desc_.depth_func)),
			ogl_front_stencil_func_(OGLMapping::Mapping(dss_desc_.front_stencil_func)),
			ogl_front_stencil_fail_(OGLMapping::Mapping(dss_desc_.front_stencil_fail)),
			ogl_front_stencil_depth_fail_(OGLMapping::Mapping(dss_desc_.front_stencil_depth_fail)),
			ogl_front_stencil_pass_(OGLMapping::Mapping(dss_desc_.front_stencil_pass)),
			ogl_back_stencil_func_(OGLMapping::Mapping(dss_desc_.back_stencil_func)),
			ogl_back_stencil_fail_(OGLMapping::Mapping(dss_desc_.back_stencil_fail)),
			ogl_back_stencil_depth_fail_(OGLMapping::Mapping(dss_desc_.back_stencil_depth_fail)),
			ogl_back_stencil_pass_(OGLMapping::Mapping(dss_desc_.back_stencil_pass)),
			ogl_blend_op_(OGLMapping::Mapping(bs_desc_.blend_op[0])),
			ogl_blend_op_alpha_(OGLMapping::Mapping(bs_desc_.blend_op_alpha[0])),
			ogl_src_blend_(OGLMapping::Mapping(bs_desc_.src_blend[0])),
			ogl_dest_blend_(OGLMapping::Mapping(bs_desc_.dest_blend[0])),
			ogl_src_blend_alpha_(OGLMapping::Mapping(bs_desc_.src_blend_alpha[0])),
			ogl_dest_blend_alpha_(OGLMapping::Mapping(bs_desc_.dest_blend_alpha[0])),
			ogl_logic_op_(OGLMapping::Mapping(bs_desc_.logic_op[0]))
	{
	}

	void OGLRenderStateObject::Active()
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		auto const & rs_obj = re.CurRenderStateObject();
		RasterizerStateDesc const & cur_rs_desc = rs_obj->GetRasterizerStateDesc();
		DepthStencilStateDesc const & cur_dss_desc = rs_obj->GetDepthStencilStateDesc();
		BlendStateDesc const & cur_bs_desc = rs_obj->GetBlendStateDesc();

		re.SetPolygonMode(GL_FRONT_AND_BACK, ogl_polygon_mode_);
		if (cur_rs_desc.shade_mode != rs_desc_.shade_mode)
		{
			glShadeModel(ogl_shade_mode_);
		}
		if (cur_rs_desc.cull_mode != rs_desc_.cull_mode)
		{
			switch (rs_desc_.cull_mode)
			{
			case CM_None:
				glDisable(GL_CULL_FACE);
				break;

			case CM_Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;

			case CM_Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}
		}

		if (cur_rs_desc.front_face_ccw != rs_desc_.front_face_ccw)
		{
			glFrontFace(ogl_front_face_);
		}

		if ((cur_rs_desc.polygon_offset_factor != rs_desc_.polygon_offset_factor)
			|| (cur_rs_desc.polygon_offset_units != rs_desc_.polygon_offset_units))
		{
			// Bias is in {0, 16}, scale the unit addition appropriately
			glPolygonOffset(rs_desc_.polygon_offset_factor, rs_desc_.polygon_offset_units);
		}

		if (cur_rs_desc.depth_clip_enable != rs_desc_.depth_clip_enable)
		{
			if (glloader_GL_VERSION_3_2() || glloader_GL_ARB_depth_clamp())
			{
				if (rs_desc_.depth_clip_enable)
				{
					glDisable(GL_DEPTH_CLAMP);
				}
				else
				{
					glEnable(GL_DEPTH_CLAMP);
				}
			}
		}

		if (cur_rs_desc.scissor_enable != rs_desc_.scissor_enable)
		{
			if (rs_desc_.scissor_enable)
			{
				glEnable(GL_SCISSOR_TEST);
			}
			else
			{
				glDisable(GL_SCISSOR_TEST);
			}
		}

		if (cur_rs_desc.multisample_enable != rs_desc_.multisample_enable)
		{
			if (rs_desc_.multisample_enable)
			{
				glEnable(GL_MULTISAMPLE);
			}
			else
			{
				glDisable(GL_MULTISAMPLE);
			}
		}

		if (cur_dss_desc.depth_enable != dss_desc_.depth_enable)
		{
			if (dss_desc_.depth_enable)
			{
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}
		}
		if (cur_dss_desc.depth_write_mask != dss_desc_.depth_write_mask)
		{
			glDepthMask(ogl_depth_write_mask_);
		}
		if (cur_dss_desc.depth_func != dss_desc_.depth_func)
		{
			glDepthFunc(ogl_depth_func_);
		}

		if ((cur_dss_desc.front_stencil_func != dss_desc_.front_stencil_func)
			|| (cur_dss_desc.front_stencil_ref != dss_desc_.front_stencil_ref)
			|| (cur_dss_desc.front_stencil_read_mask != dss_desc_.front_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_FRONT, ogl_front_stencil_func_,
				dss_desc_.front_stencil_ref, dss_desc_.front_stencil_read_mask);
		}
		if ((cur_dss_desc.front_stencil_fail != dss_desc_.front_stencil_fail)
			|| (cur_dss_desc.front_stencil_depth_fail != dss_desc_.front_stencil_depth_fail)
			|| (cur_dss_desc.front_stencil_pass != dss_desc_.front_stencil_pass))
		{
			glStencilOpSeparate(GL_FRONT, ogl_front_stencil_fail_,
				ogl_front_stencil_depth_fail_, ogl_front_stencil_pass_);
		}
		if (cur_dss_desc.front_stencil_write_mask != dss_desc_.front_stencil_write_mask)
		{
			glStencilMaskSeparate(GL_FRONT, dss_desc_.front_stencil_write_mask);
		}

		if ((cur_dss_desc.back_stencil_func != dss_desc_.back_stencil_func)
			|| (cur_dss_desc.back_stencil_ref != dss_desc_.back_stencil_ref)
			|| (cur_dss_desc.back_stencil_read_mask != dss_desc_.back_stencil_read_mask))
		{
			glStencilFuncSeparate(GL_BACK, ogl_back_stencil_func_,
				dss_desc_.back_stencil_ref, dss_desc_.back_stencil_read_mask);
		}
		if ((cur_dss_desc.back_stencil_fail != dss_desc_.back_stencil_fail)
			|| (cur_dss_desc.back_stencil_depth_fail != dss_desc_.back_stencil_depth_fail)
			|| (cur_dss_desc.back_stencil_pass != dss_desc_.back_stencil_pass))
		{
			glStencilOpSeparate(GL_BACK, ogl_back_stencil_fail_,
				ogl_back_stencil_depth_fail_, ogl_back_stencil_pass_);
		}
		if (cur_dss_desc.back_stencil_write_mask != dss_desc_.back_stencil_write_mask)
		{
			glStencilMaskSeparate(GL_BACK, dss_desc_.back_stencil_write_mask);
		}

		if ((cur_dss_desc.front_stencil_enable != dss_desc_.front_stencil_enable)
			|| (cur_dss_desc.back_stencil_enable != dss_desc_.back_stencil_enable))
		{
			if (dss_desc_.front_stencil_enable || dss_desc_.back_stencil_enable)
			{
				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}

		if (cur_bs_desc.alpha_to_coverage_enable != bs_desc_.alpha_to_coverage_enable)
		{
			if (bs_desc_.alpha_to_coverage_enable)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
		}
		if (bs_desc_.independent_blend_enable)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (cur_bs_desc.blend_enable[i] != bs_desc_.blend_enable[i])
				{
					if (bs_desc_.blend_enable[i])
					{
						glEnablei(GL_BLEND, i);
					}
					else
					{
						glDisablei(GL_BLEND, i);
					}
				}
			}
		}
		else
		{
			if (cur_bs_desc.blend_enable[0] != bs_desc_.blend_enable[0])
			{
				if (bs_desc_.blend_enable[0])
				{
					glEnable(GL_BLEND);
				}
				else
				{
					glDisable(GL_BLEND);
				}
			}
		}

		if (cur_bs_desc.logic_op_enable[0] != bs_desc_.logic_op_enable[0])
		{
			if (bs_desc_.logic_op_enable[0])
			{
				glEnable(GL_LOGIC_OP_MODE);
			}
			else
			{
				glDisable(GL_LOGIC_OP_MODE);
			}
		}

		if (glloader_GL_VERSION_4_0())
		{
			for (int i = 0; i < 8; ++i)
			{
				if (cur_bs_desc.blend_op[i] != bs_desc_.blend_op[i])
				{
					glBlendEquationSeparatei(i, ogl_blend_op_, ogl_blend_op_alpha_);
				}
				if ((cur_bs_desc.src_blend[i] != bs_desc_.src_blend[i])
					|| (cur_bs_desc.dest_blend[i] != bs_desc_.dest_blend[i])
					|| (cur_bs_desc.src_blend_alpha[i] != bs_desc_.src_blend_alpha[i])
					|| (cur_bs_desc.dest_blend_alpha[i] != bs_desc_.dest_blend_alpha[i]))
				{
					glBlendFuncSeparatei(i, ogl_src_blend_, ogl_dest_blend_,
						ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
				}
			}
		}
		else if (glloader_GL_ARB_draw_buffers_blend())
		{
			for (int i = 0; i < 8; ++i)
			{
				if (cur_bs_desc.blend_op[i] != bs_desc_.blend_op[i])
				{
					glBlendEquationSeparateiARB(i, ogl_blend_op_, ogl_blend_op_alpha_);
				}
				if ((cur_bs_desc.src_blend[i] != bs_desc_.src_blend[i])
					|| (cur_bs_desc.dest_blend[i] != bs_desc_.dest_blend[i])
					|| (cur_bs_desc.src_blend_alpha[i] != bs_desc_.src_blend_alpha[i])
					|| (cur_bs_desc.dest_blend_alpha[i] != bs_desc_.dest_blend_alpha[i]))
				{
					glBlendFuncSeparateiARB(i, ogl_src_blend_, ogl_dest_blend_,
						ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
				}
			}
		}
		else
		{
			if (cur_bs_desc.blend_op[0] != bs_desc_.blend_op[0])
			{
				glBlendEquationSeparate(ogl_blend_op_, ogl_blend_op_alpha_);
			}
			if ((cur_bs_desc.src_blend[0] != bs_desc_.src_blend[0])
				|| (cur_bs_desc.dest_blend[0] != bs_desc_.dest_blend[0])
				|| (cur_bs_desc.src_blend_alpha[0] != bs_desc_.src_blend_alpha[0])
				|| (cur_bs_desc.dest_blend_alpha[0] != bs_desc_.dest_blend_alpha[0]))
			{
				glBlendFuncSeparate(ogl_src_blend_, ogl_dest_blend_,
					ogl_src_blend_alpha_, ogl_dest_blend_alpha_);
			}
		}
		if (bs_desc_.independent_blend_enable)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (cur_bs_desc.color_write_mask[i] != bs_desc_.color_write_mask[i])
				{
					glColorMaski(i, (bs_desc_.color_write_mask[i] & CMASK_Red) != 0,
						(bs_desc_.color_write_mask[i] & CMASK_Green) != 0,
						(bs_desc_.color_write_mask[i] & CMASK_Blue) != 0,
						(bs_desc_.color_write_mask[i] & CMASK_Alpha) != 0);
				}
			}
		}
		else
		{
			if (cur_bs_desc.color_write_mask[0] != bs_desc_.color_write_mask[0])
			{
				glColorMask((bs_desc_.color_write_mask[0] & CMASK_Red) != 0,
					(bs_desc_.color_write_mask[0] & CMASK_Green) != 0,
					(bs_desc_.color_write_mask[0] & CMASK_Blue) != 0,
					(bs_desc_.color_write_mask[0] & CMASK_Alpha) != 0);
			}
		}

		if (cur_bs_desc.logic_op[0] != bs_desc_.logic_op[0])
		{
			glLogicOp(ogl_logic_op_);
		}

		if (cur_bs_desc.blend_factor != bs_desc_.blend_factor)
		{
			glBlendColor(bs_desc_.blend_factor.r(), bs_desc_.blend_factor.g(), bs_desc_.blend_factor.b(), bs_desc_.blend_factor.a());
		}
	}

	void OGLRenderStateObject::ForceDefaultState()
	{
		RasterizerStateDesc rs_desc;
		DepthStencilStateDesc dss_desc;
		BlendStateDesc bs_desc;

		glPolygonMode(GL_FRONT_AND_BACK, OGLMapping::Mapping(rs_desc.polygon_mode));
		glShadeModel(OGLMapping::Mapping(rs_desc.shade_mode));
		switch (rs_desc.cull_mode)
		{
		case CM_None:
			glDisable(GL_CULL_FACE);
			break;

		case CM_Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;

		case CM_Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		}

		glFrontFace(rs_desc.front_face_ccw ? GL_CCW : GL_CW);

		glPolygonOffset(rs_desc.polygon_offset_factor, rs_desc.polygon_offset_units);

		if (glloader_GL_VERSION_3_2() || glloader_GL_ARB_depth_clamp())
		{
			if (rs_desc.depth_clip_enable)
			{
				glDisable(GL_DEPTH_CLAMP);
			}
			else
			{
				glEnable(GL_DEPTH_CLAMP);
			}
		}

		if (rs_desc.scissor_enable)
		{
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}

		if (rs_desc.multisample_enable)
		{
			glEnable(GL_MULTISAMPLE);
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
		}

		if (dss_desc.depth_enable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		glDepthMask(dss_desc.depth_write_mask ? GL_TRUE : GL_FALSE);
		glDepthFunc(OGLMapping::Mapping(dss_desc.depth_func));

		glStencilFuncSeparate(GL_FRONT, OGLMapping::Mapping(dss_desc.front_stencil_func),
			0, dss_desc.front_stencil_read_mask);
		glStencilOpSeparate(GL_FRONT, OGLMapping::Mapping(dss_desc.front_stencil_fail),
			OGLMapping::Mapping(dss_desc.front_stencil_depth_fail), OGLMapping::Mapping(dss_desc.front_stencil_pass));
		glStencilMaskSeparate(GL_FRONT, dss_desc.front_stencil_write_mask);

		glStencilFuncSeparate(GL_BACK, OGLMapping::Mapping(dss_desc.back_stencil_func),
			0, dss_desc.back_stencil_read_mask);
		glStencilOpSeparate(GL_BACK, OGLMapping::Mapping(dss_desc.back_stencil_fail),
			OGLMapping::Mapping(dss_desc.back_stencil_depth_fail), OGLMapping::Mapping(dss_desc.back_stencil_pass));
		glStencilMaskSeparate(GL_BACK, dss_desc.back_stencil_write_mask);

		if (dss_desc.front_stencil_enable || dss_desc.back_stencil_enable)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}

		if (bs_desc.alpha_to_coverage_enable)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		else
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		if (bs_desc.blend_enable[0])
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		if (bs_desc.logic_op_enable[0])
		{
			glEnable(GL_COLOR_LOGIC_OP);
		}
		else
		{
			glDisable(GL_COLOR_LOGIC_OP);
		}
		glBlendEquationSeparate(OGLMapping::Mapping(bs_desc.blend_op[0]), OGLMapping::Mapping(bs_desc.blend_op_alpha[0]));
		glBlendFuncSeparate(OGLMapping::Mapping(bs_desc.src_blend[0]), OGLMapping::Mapping(bs_desc.dest_blend[0]),
			OGLMapping::Mapping(bs_desc.src_blend_alpha[0]), OGLMapping::Mapping(bs_desc.dest_blend_alpha[0]));
		glColorMask((bs_desc.color_write_mask[0] & CMASK_Red) != 0,
			(bs_desc.color_write_mask[0] & CMASK_Green) != 0,
			(bs_desc.color_write_mask[0] & CMASK_Blue) != 0,
			(bs_desc.color_write_mask[0] & CMASK_Alpha) != 0);
		glLogicOp(OGLMapping::Mapping(bs_desc.logic_op[0]));

		glBlendColor(1, 1, 1, 1);
	}


	OGLSamplerStateObject::OGLSamplerStateObject(SamplerStateDesc const & desc)
		: SamplerStateObject(desc),
			ogl_addr_mode_u_(OGLMapping::Mapping(desc_.addr_mode_u)),
			ogl_addr_mode_v_(OGLMapping::Mapping(desc_.addr_mode_v)),
			ogl_addr_mode_w_(OGLMapping::Mapping(desc_.addr_mode_w))
	{
		if (desc_.filter & TFOE_Min_Linear)
		{
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				ogl_min_filter_ = GL_LINEAR_MIPMAP_NEAREST;
			}
		}
		else
		{
			if (desc_.filter & TFOE_Mip_Linear)
			{
				ogl_min_filter_ = GL_NEAREST_MIPMAP_LINEAR;
			}
			else
			{
				ogl_min_filter_ = GL_NEAREST_MIPMAP_NEAREST;
			}
		}
		if (desc_.filter & TFOE_Mag_Linear)
		{
			ogl_mag_filter_ = GL_LINEAR;
		}
		else
		{
			ogl_mag_filter_ = GL_NEAREST;
		}
		if (desc_.filter & TFOE_Anisotropic)
		{
			ogl_mag_filter_ = GL_LINEAR;
			ogl_min_filter_ = GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	void OGLSamplerStateObject::Active(TexturePtr const & texture)
	{
		OGLTexture& tex = *checked_cast<OGLTexture*>(texture.get());

		tex.TexParameteri(GL_TEXTURE_WRAP_S, ogl_addr_mode_u_);
		tex.TexParameteri(GL_TEXTURE_WRAP_T, ogl_addr_mode_v_);
		tex.TexParameteri(GL_TEXTURE_WRAP_R, ogl_addr_mode_w_);

		tex.TexParameterfv(GL_TEXTURE_BORDER_COLOR, &desc_.border_clr.r());

		tex.TexParameteri(GL_TEXTURE_MAG_FILTER, ogl_mag_filter_);
		tex.TexParameteri(GL_TEXTURE_MIN_FILTER, ogl_min_filter_);

		if (glloader_GL_EXT_texture_filter_anisotropic())
		{
			if (desc_.filter & TFOE_Anisotropic)
			{
				tex.TexParameteri(GL_TEXTURE_MAX_ANISOTROPY_EXT, desc_.max_anisotropy);
			}
			else
			{
				tex.TexParameteri(GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			}
		}
		tex.TexParameterf(GL_TEXTURE_MIN_LOD, desc_.min_lod);
		tex.TexParameterf(GL_TEXTURE_MAX_LOD, desc_.max_lod);
		if (desc_.cmp_func != CF_AlwaysFail)
		{
			tex.TexParameteri(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		}
		else
		{
			tex.TexParameteri(GL_TEXTURE_COMPARE_MODE, GL_NONE);
		}
		tex.TexParameteri(GL_TEXTURE_COMPARE_FUNC, OGLMapping::Mapping(desc_.cmp_func));

		tex.TexParameterf(GL_TEXTURE_LOD_BIAS, desc_.mip_map_lod_bias);
	}
}
