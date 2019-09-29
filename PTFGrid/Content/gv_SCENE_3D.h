//                      
//                      
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                      
//               ghv : Garrett Vance : 2018_06_07
//                      
//                      
//          Implementation of Parallel Transport Frames
//		    following Andrew J. Hanson's paper "Parallel 
//		    Transport Approach to Curve Framing" of 1995. 
//                      
//                      
//      DirectX 11 Application for Windows 10 Universal Windows
//                      
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                      

 
#pragma once


#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"





#undef GHV_OPTION_TREFOIL_KNOT
#undef GHV_OPTION_DRAW_SPOKES
#define GHV_OPTION_ANIMATE_CURVE_EVOLUTION







namespace VHG
{

    struct VHG_Axonodromal_Vertex
    {
        DirectX::XMFLOAT3       axon_position_r;
        float                   axon_elapsed_time;
        DirectX::XMFLOAT3       axon_tangent_drdt;
        DirectX::XMFLOAT3       axon_d2rdt2;

    };




    enum class MeshObjectShape
    {
        Lissajous,
        Spiral,
        ParabolicHelix,
        Corkscrew
    }; 



    class VHG_Scene3D1
    {
    public:
        VHG_Scene3D1(const std::shared_ptr<DX::DeviceResources>& deviceResources);


        void VHG_Scene3D1::Load_Texture_Triax(void); 
        void VHG_Scene3D1::Create_Rasterizer_State(void); 
        void VHG_Scene3D1::Create_Input_Layout(
            const std::vector<byte>& p_byte_vector
        ); 
        void VHG_Scene3D1::Create_Vertex_Buffer(
            std::vector<VHG_VertexPosTex>  *p_vect_vertices,
            ID3D11Buffer** p_buffer_object
        );



        void VHG_Scene3D1::HansonParallelTransportFrame(void); 

        size_t VHG_Scene3D1::gv_load_trefoil_knot(
                std::vector<VHG_Axonodromal_Vertex>    *p_space_curve_points
        );

		void VHG_Scene3D1::gv_finite_differences(
			std::vector<VHG_Axonodromal_Vertex>    *p_curve_derivatives
		);






        void VHG_Scene3D1::gv_next_mesh_object_shape(void); 
        void VHG_Scene3D1::do_rotation_y_axis(DirectX::XMMATRIX&  p_rot_mat); 
        uint8_t     VHG_Scene3D1::get_angular_m(void)
        {
            return    e_angular_m;
        }
        uint8_t     VHG_Scene3D1::get_angular_n(void)
        {
            return    e_angular_n;
        }

        uint32_t     VHG_Scene3D1::Get_Vertex_Count_Triax(void)
        {
            return    m_vertex_buffer_1_count;
        }

        void VHG_Scene3D1::animator_update(void); 
       
        uint32_t     VHG_Scene3D1::gv_GetAnimatorCount(void)
        {
            return m_WVP_constant_buffer_data.animator_count.x;
        }

        uint32_t     VHG_Scene3D1::gv_get_advert_modulus(void)
        {
            return e_advert_modulus;
        }

        void VHG_Scene3D1::GV_Camera_Init(void);
        float VHG_Scene3D1::GV_Get_Yaw(void) const { return e_yaw; }
        float VHG_Scene3D1::GV_Get_Pitch(void) const { return e_pitch; }

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();

        void Update(DX::StepTimer const& timer);
        void Render();
    private: 
        std::shared_ptr<DX::DeviceResources>            m_deviceResources;
        Microsoft::WRL::ComPtr<ID3D11Buffer>            m_WVP_constantBuffer;
        VHG_ConBuf_MVP_Struct                           m_WVP_constant_buffer_data;



        //    now i have TWO distinct Vertex Buffer objects: 

        Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertex_buffer_1_buffer;
        uint32_t                                        m_vertex_buffer_1_count;

        Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertex_buffer_2_buffer;
        uint32_t                                        m_vertex_buffer_2_count;


        uint32_t                                        e_axon_arc_density;
        uint32_t                                        tube_facets;
        float                                           tube_radius;
        uint32_t                                        m_vtxbuf_2_draw_count;


        Microsoft::WRL::ComPtr<ID3D11VertexShader>      t_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>       t_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>       t_inputLayout;

        bool                                            m_loadingComplete;
        float                                           m_degreesPerSecond;
        uint32_t                                        e_advert_modulus;
    
        MeshObjectShape                                 e_mesh_object_shape; 
        bool                                            e_option_rotate_lorenz;
        bool                                            e_option_flip_x;
        uint8_t                                         e_angular_m;    
        uint8_t                                         e_angular_n;   

        std::unique_ptr<DirectX::Keyboard>              e_keyboard;
        std::unique_ptr<DirectX::Mouse>                 e_mouse; 
        float                                           e_pitch;
        float                                           e_yaw; 
        DirectX::SimpleMath::Vector3                    e_camera_position;
        DirectX::SimpleMath::Matrix                     e_view_matrix;
        DirectX::SimpleMath::Matrix                     e_projection_matrix;

        Microsoft::WRL::ComPtr<ID3D11SamplerState>          e_texture_sampler_state;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    e_texture_srv;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>       e_rasterizer_state;
        D3D11_FILL_MODE                                     e_rasterizer_fill_mode;
    };
	//  Closes class VHG_Scene3D1; 

}
//  Closes namespace VHG; 



                                                                
