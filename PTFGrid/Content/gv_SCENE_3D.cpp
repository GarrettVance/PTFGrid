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

#include "pch.h"
#include "gv_SCENE_3D.h"
#include "..\Common\DirectXHelper.h"

#include <fstream>   //  for LORENZ ATTRACTOR datafile reader std::fstream
#include <string>    //  for LORENZ ATTRACTOR datafile reader std::getline
#include <sstream>   //  for LORENZ ATTRACTOR datafile reader std::istringstream
#include <iterator>  //  for LORENZ ATTRACTOR datafile reader std::istream_iterator


//    CWS : Link to Chuck Walbourn's DirectXTK library: 

#pragma comment(lib, "DirectXTK")


using namespace VHG;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Windows::Foundation;


bool g0_Button_Processed = false; 


VHG_Scene3D1::VHG_Scene3D1(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources),  
    m_loadingComplete(false),
    e_rasterizer_fill_mode(D3D11_FILL_WIREFRAME),
    //   e_mesh_object_shape(MeshObjectShape::Corkscrew),
    e_mesh_object_shape(MeshObjectShape::Spiral),
    e_option_rotate_lorenz(false),
    e_option_flip_x(true),
    e_angular_m(4),
    e_angular_n(3),
    e_pitch(0),
    e_yaw(0), 
    e_advert_modulus(21),
    m_degreesPerSecond(45)
{  
    //      Configure the tubular extrusion:

#ifdef GHV_OPTION_TREFOIL_KNOT

    // originally : this->e_axon_arc_density = 256;   //  values 256, 512, 1024...

	this->e_axon_arc_density = 128; 

    this->tube_radius = 0.14f;  // Spoke length and tube radius > 0.01. 

#else

    //      Lorenz Attractor
    //  this->e_axon_arc_density = 8192;   //  GOOD value;
    
    this->e_axon_arc_density = 16384;   //  

    this->tube_radius = 0.05f;      // Spoke length and tube radius > 0.01. 

#endif


    this->tube_facets = 6;
    UINT tube_quantum = (3 + 3) * tube_facets;
    m_vtxbuf_2_draw_count = 5 * tube_quantum;

    e_keyboard = std::make_unique<DirectX::Keyboard>();
    e_keyboard->SetWindow(Windows::UI::Core::CoreWindow::GetForCurrentThread());
    e_mouse = std::make_unique<DirectX::Mouse>();
    e_mouse->SetWindow(Windows::UI::Core::CoreWindow::GetForCurrentThread());


    //  ctor set m_WVP_...data to zeros initially

    this->m_WVP_constantBuffer = nullptr;
    XMMATRIX  my_zero_matrix = 0.f * XMMatrixIdentity(); 
    XMFLOAT4X4 my_zero_4x4; 
    XMStoreFloat4x4(&my_zero_4x4, my_zero_matrix);
    DirectX::XMUINT4   zero_ui4(0, 0, 0, 0); 
    this->m_WVP_constant_buffer_data = {my_zero_4x4, my_zero_4x4, my_zero_4x4, zero_ui4};


    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}  
//  Closes class ctor;  






void VHG_Scene3D1::CreateWindowSizeDependentResources()
{ 
    // Initializes view parameters when the window size changes.
    
    Size outputSize = m_deviceResources->GetOutputSize(); 

    float aspectRatio = outputSize.Width / outputSize.Height;  

    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // Note that the OrientationTransform3D matrix is post-multiplied here
    // in order to correctly orient the scene to match the display orientation.
    // This post-multiplication step is required for any draw calls that are
    // made to the swap chain render target. For draw calls to other targets,
    // this transform should not be applied.


    // 
    //    DirectXMath offers both left-handed and right-handed versions 
    //          of matrix functions with 'handedness', 
    //          but always assumes a row-major format.
    // 

    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        0.01f,
        850.0f   //  originally 100.
    );

    XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
    XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

    e_projection_matrix = perspectiveMatrix * orientationMatrix;

    XMStoreFloat4x4(
        &m_WVP_constant_buffer_data.projection,
        XMMatrixTranspose(e_projection_matrix)
    );

    GV_Camera_Init();
}  
//  Closes VHG_Scene3D1::CreateWindowSizeDependentResources;  


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Render()  
{ 
    if (!m_loadingComplete)
    {
        return;
    }

    static uint32_t idx_waiting = 0; 
    if (g0_Button_Processed)
    {
        idx_waiting++; 
        if (idx_waiting > 90)   //  GOLD : 120 
        {
            //    Wait some duration 
            //    in order to debounce the keyboard. 
            //    If frame rate is 60 render calls per second, 
            //    then waiting 120 loops is 2 seconds lock-out
            //    on the keyboard...

            idx_waiting = 0; 
            g0_Button_Processed = false;
        }
    }

    auto context = m_deviceResources->GetD3DDeviceContext();


    // Send WVP data to constant buffer

    context->UpdateSubresource1(
            m_WVP_constantBuffer.Get(),   0,  NULL, 
            &m_WVP_constant_buffer_data,    0,   0,  0 );


    context->VSSetConstantBuffers1(
            0,   1,   m_WVP_constantBuffer.GetAddressOf(), 
            nullptr,  nullptr );


    Create_Rasterizer_State();
    context->RSSetState(e_rasterizer_state.Get());



    context->IASetInputLayout(t_inputLayout.Get());
    
    context->VSSetShader(t_vertexShader.Get(), nullptr, 0);
   
    context->PSSetShader(t_pixelShader.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, e_texture_srv.GetAddressOf());
    context->PSSetSamplers(0, 1, e_texture_sampler_state.GetAddressOf());


#ifdef GHV_OPTION_DRAW_SPOKES
    UINT vertex_buffer_1_stride = sizeof(VHG_VertexPosTex);
    UINT vertex_buffer_1_offset = 0;
    context->IASetVertexBuffers(
        0,
        1,
        m_vertex_buffer_1_buffer.GetAddressOf(),
        &vertex_buffer_1_stride,
        &vertex_buffer_1_offset
    );
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->Draw(this->m_vertex_buffer_1_count, 0);
#endif

    UINT vertex_buffer_2_stride = sizeof(VHG_VertexPosTex);
    UINT vertex_buffer_2_offset = 0;
    context->IASetVertexBuffers(
        0,
        1,
        m_vertex_buffer_2_buffer.GetAddressOf(),
        &vertex_buffer_2_stride,
        &vertex_buffer_2_offset
    );
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //      The contents of Vertex Buffer 2
    //      (the triangle mesh for the tube surface)
    //      has cardinality given by 
    //      card = (-1 + g0_curvepoints) * g0_faces * (3 + 3).
    //
    //      The factor (3 + 3) corresponds to the pair of triangles
    //      created for each face.
    //          
    //      Portions of the tube then should be rendered 
    //      as multiples of (g0_faces * 6).

    const UINT a_vertex_multiple = 6 * tube_facets;

    UINT a_start_index_1 = 0;
    assert(a_start_index_1 + this->m_vtxbuf_2_draw_count < 1 + m_vertex_buffer_2_count); 

    context->Draw(
        m_vtxbuf_2_draw_count, // Vertex Count 
        a_start_index_1   // Starting Index of Vertex to Draw 
    );


#ifdef GHV_OPTION_ANIMATE_CURVE_EVOLUTION
    //      Use two start_index values:
    //      One to begin at zero (the head of the Vertex Buffer);
    //      The other to start halfway through the Vertex Buffer.

    UINT a_start_index_2 = a_vertex_multiple * (UINT)floor((e_axon_arc_density - 1) / 2.f);
    assert(a_start_index_2 + this->m_vtxbuf_2_draw_count < 1 + m_vertex_buffer_2_count); 
    context->Draw(
        m_vtxbuf_2_draw_count, // Vertex Count 
        a_start_index_2   // Starting Index of Vertex to Draw 
    );
#endif
}  
//  Closes VHG_Scene3D1::Render;  


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Create_Vertex_Buffer(
    std::vector<VHG_VertexPosTex>  *p_vect_vertices,
    ID3D11Buffer** p_buffer_object
)
{ 
    size_t   bytes_required_allocation =
        sizeof(VHG_VertexPosTex) * p_vect_vertices->size();

    const VHG_VertexPosTex   *const_data_ptr = &(*p_vect_vertices)[0];


    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
    vertexBufferData.pSysMem = const_data_ptr;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC vertexBufferDesc(
        bytes_required_allocation,   // the total required allocation;  
        D3D11_BIND_VERTEX_BUFFER
    );

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexBufferData,
            p_buffer_object
        )
    );
}  
//  Closes VHG_Scene3D1::Create_Vertex_Buffer; 







//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::gv_finite_differences(
	std::vector<VHG_Axonodromal_Vertex>    *p_axonodromal_vertices
)
{
	double const e_underflow = 0.0000000001;

	for (UINT idx_nodes = 0; idx_nodes < e_axon_arc_density; idx_nodes++)
	{

		//   
        //    Obtain an approximate first derivative dr/dt 
		//    (aka tangent vector) via finite difference techniques:
		//               

		UINT idx_neung = idx_nodes;

		UINT idx_song = (idx_nodes == (-1 + e_axon_arc_density)) ? 0 : 1 + idx_nodes;

		//  assert the space curve is a closed loop (ouroboros); 


		double dt =
			p_axonodromal_vertices->at(idx_song).axon_elapsed_time -
			p_axonodromal_vertices->at(idx_neung).axon_elapsed_time; 

		if (abs(dt) < e_underflow)
		{
			if (dt < 0.00) dt = -1.00 * e_underflow;
			else if (dt > 0.00) dt = e_underflow;
		}


		double tangent_drdt_x =
			(p_axonodromal_vertices->at(idx_song).axon_position_r.x -
			 p_axonodromal_vertices->at(idx_neung).axon_position_r.x) / dt;

		double tangent_drdt_y =
			(p_axonodromal_vertices->at(idx_song).axon_position_r.y -
			 p_axonodromal_vertices->at(idx_neung).axon_position_r.y) / dt;

		double tangent_drdt_z =
			(p_axonodromal_vertices->at(idx_song).axon_position_r.z -
			 p_axonodromal_vertices->at(idx_neung).axon_position_r.z) / dt;


			
		p_axonodromal_vertices->at(idx_neung).axon_tangent_drdt = DirectX::XMFLOAT3(
			(float)tangent_drdt_x,
			(float)tangent_drdt_y,
			(float)tangent_drdt_z
		);

	}
}
//  Closes VHG_Scene3D1::gv_finite_differences();  






size_t VHG_Scene3D1::gv_load_trefoil_knot(
    std::vector<VHG_Axonodromal_Vertex>    *p_curve_derivatives
)
{
    VHG_Axonodromal_Vertex    tmp_curve_derivative;

    for (UINT idx_nodes = 0; idx_nodes < e_axon_arc_density; idx_nodes++)
    {
        //          Trefoil Knot
        
        double t_parameter = 2.00 * DirectX::XM_PI * idx_nodes / (double)(-1 + e_axon_arc_density);


        double x_coord = sin(t_parameter) + 2.00 * sin(2.00 * t_parameter);
        double y_coord = cos(t_parameter) - 2.00 * cos(2.00 * t_parameter);
        double z_coord = -1.00 * sin(3.00 * t_parameter);
     

        tmp_curve_derivative = 
        { 
            XMFLOAT3((float)x_coord, (float)y_coord, (float)z_coord), 
            (float)t_parameter,
            XMFLOAT3(0.f, 0.f, 0.f)
        };
        p_curve_derivatives->push_back(tmp_curve_derivative);
    }

	gv_finite_differences(p_curve_derivatives);

    return p_curve_derivatives->size();
}
//  Closes VHG_Scene3D1::gv_load_trefoil_knot();







//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++








template<typename T>
std::vector<T> gv_split_n(const std::string& line) {
	std::istringstream is(line);
	return std::vector<T>(std::istream_iterator<T>(is), std::istream_iterator<T>());
}








//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifndef GHV_OPTION_TREFOIL_KNOT

size_t gv_read_lorenz_data_file(
    std::vector<VHG_Axonodromal_Vertex>    *p_space_curve_points,
    uint32_t p_max_records
)
{  
	//    Lorenz data file is located under "Appx\Assets": 

	std::fstream gFStr("Assets\\lorenz.ghvdata", std::ios_base::in);

	if (!gFStr.is_open()) { return 0; }

	std::string g_line = ""; 
	VHG_Axonodromal_Vertex    tmp_curve;

	while (std::getline(gFStr, g_line))
	{
        //             Fields in data file:
        //      ==================================
        //      position_x, position_y, position_z
        //      elapsed_time
        //      dr/dt__x, dr/dt__y, dr/dt__z
        //      d2r/dt2__x, d2r/dt2__y; d2r/dt2__z
        //      ==================================

        std::vector<double> fields_split = gv_split_n<double>(g_line);

		XMFLOAT3   tmp_position = XMFLOAT3( 
                static_cast<float>(fields_split[0]), 
                static_cast<float>(fields_split[1]), 
                static_cast<float>(fields_split[2])
        );

        float tmp_time = static_cast<float>(fields_split[3]);

        XMFLOAT3   tmp_drdt = XMFLOAT3( 
                static_cast<float>(fields_split[4]), 
                static_cast<float>(fields_split[5]), 
                static_cast<float>(fields_split[6])
        );

        XMFLOAT3   tmp_d2rdt2 = XMFLOAT3( 
                static_cast<float>(fields_split[7]), 
                static_cast<float>(fields_split[8]), 
                static_cast<float>(fields_split[9])
        );

		tmp_curve = { tmp_position, tmp_time, tmp_drdt, tmp_d2rdt2 };
		p_space_curve_points->push_back(tmp_curve);

        if (p_space_curve_points->size() >= p_max_records)
        {
            break;
        }
	}
	
	gFStr.close(); //           CLOSE THE FILE !!!!! 

    return p_space_curve_points->size();
}  
//  Closes gv_read_lorenz_data_file();
#endif




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





void VHG_Scene3D1::HansonParallelTransportFrame(void)
{
    std::vector<VHG_Axonodromal_Vertex>   *curve_derivatives = new std::vector<VHG_Axonodromal_Vertex>;


    //    merge vectors  unit_tangent and transported_normal into one vector

    std::vector<XMVECTOR> *node_unit_tangent = new std::vector<XMVECTOR>();
    std::vector<XMVECTOR> *transported_normal = new std::vector<XMVECTOR>();


    std::vector<VHG_VertexPosTex>   *surface_points = new std::vector<VHG_VertexPosTex>;
    std::vector<VHG_VertexPosTex>   *spoke_lines = new std::vector<VHG_VertexPosTex>;
    std::vector<VHG_VertexPosTex>   *surface_triangles = new std::vector<VHG_VertexPosTex>;


#ifdef GHV_OPTION_TREFOIL_KNOT
    size_t record_count = gv_load_trefoil_knot(
            curve_derivatives
    );
#else
    size_t record_count = gv_read_lorenz_data_file(
            curve_derivatives, 
            e_axon_arc_density  /* p_max_records */
    );
#endif

    for (UINT idx_nodes = 0; idx_nodes < e_axon_arc_density; idx_nodes++)
    {
        XMVECTOR drdt_as_xmvector = XMLoadFloat3(& (curve_derivatives->at(idx_nodes).axon_tangent_drdt) );

        //   normalize the drdt vector to obtain unit tangent vector;

        XMVECTOR unit_tangent_vect = XMVector3Normalize(drdt_as_xmvector);
        node_unit_tangent->push_back(unit_tangent_vect);

        if (idx_nodes == 0)
        {
            //  Special case when idx_nodes == 0: 
            //  ========================================
            //  Construct what Andrew J. Hanson terms the "initial normal vector V0": 
            //         
            //   There are many ways to construct the initial normal,... 
            // 
            //     
            //
            //   I don't think Hanson's algorithm requires
            //   the initial normal vector to be normalize to unit length,
            //   so dividing by the norm might be superfluous.
            //
            //   ....??? not sure...
			//           

			//   obtain an XMVECTOR of the vector difference 
			//   axon_vertex[1] - axon_vertex[0], call this axon_delta_r. 
			//   
			//   This is only a conceptual explanation, because the vertices
			//   of the axonodromal curve aren't stored in C-arrays at all.
			//  
			//   But the index access shown above uses hard-coded indexers 
			//   because this step of the solution is utilized ONLY once, 
			//   namely as a bootstrap to get the "initial normal vector". 
			//   


			XMVECTOR axon_tangent_zero = XMLoadFloat3( &(curve_derivatives->at(0).axon_tangent_drdt) );

			XMVECTOR axon_r_position_zero = XMLoadFloat3( &(curve_derivatives->at(0).axon_position_r) );
			XMVECTOR axon_r_position_one  = XMLoadFloat3( &(curve_derivatives->at(1).axon_position_r) );

			XMVECTOR axon_delta_r = axon_r_position_one - axon_r_position_zero;




			//  TODO: ???   Regarding the vector cross product below: 
			//  TODO: ???   should i use the finite difference derivative "axon_tangent_zero" 
			//  TODO: ???   rather than the simple delta "axon_delta_r" ???
			//  TODO: ???            I am uncertain...

			XMVECTOR g_zero = XMVector3Cross(axon_r_position_zero, axon_delta_r);  

			//  assert that g_zero is  perpendicular to the tangent at axon_r_position_zero;




			//  TODO: ???   SAME QUESTION AS ABOVE:    regarding the cross product below: 
			//  TODO: ???   should i use the finite difference derivative "axon_tangent_zero" 
			//  TODO: ???   rather than the simple delta "axon_delta_r" ???

			XMVECTOR h_abnormal_zero = XMVector3Cross(g_zero, axon_delta_r); 

            XMVECTOR h_normalized_normal = XMVector3Normalize(h_abnormal_zero);
       
            transported_normal->push_back(h_normalized_normal);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //              End of curve-specific treatment;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    for (UINT idx_frame = 0; idx_frame < -1 + e_axon_arc_density; idx_frame++)
    {
        XMVECTOR B_vector = XMVector3Cross(
            node_unit_tangent->at(idx_frame),
            node_unit_tangent->at(1 + idx_frame)
        );

        float B_length = XMVectorGetX(XMVector3Length(B_vector));
        const float epsilon_length = 0.0001f;
        XMVECTOR next_transported_normal;

        if (B_length < epsilon_length)
        {
            next_transported_normal = transported_normal->at(idx_frame);
        }
        else
        {
            XMVECTOR B_unit_vector = XMVector3Normalize(B_vector);

            XMVECTOR angle_theta_vector = XMVector3AngleBetweenNormals(
                node_unit_tangent->at(idx_frame),
                node_unit_tangent->at(1 + idx_frame)
            );
            
            float angle_theta = XMVectorGetX(angle_theta_vector);

            XMMATRIX gv_rotation_matrix = XMMatrixRotationNormal(
                    B_unit_vector, 
                    angle_theta
            ); 

            next_transported_normal = XMVector3Transform(
                transported_normal->at(idx_frame),
                gv_rotation_matrix
            );
        }
        transported_normal->push_back(next_transported_normal);
    }

    //  This concludes Hanson's Parallel Transport Frames algorithm;


    size_t ghv_debug_card_position = curve_derivatives->size(); 
    
    size_t ghv_debug_card_tangents = node_unit_tangent->size(); 
   
    size_t ghv_debug_card_transported = transported_normal->size(); 

    
    for (UINT idx_section = 0; idx_section < e_axon_arc_density; idx_section++)
    {
        XMVECTOR unit_normal = XMVector3Normalize(
            transported_normal->at(idx_section)
        );

        // 
        //     In order to orient and position the cross-section, 
        //     need another normal vector in addition to the transported_normal.
        //     
        //     I have chosen to use vector B = T cross N.
        //            

        XMVECTOR binormal = XMVector3Cross(
            node_unit_tangent->at(idx_section),
            unit_normal
        );

        //   I expect that binormal doesn't need explicit normalization;
        //           
        //   The fact that node_unit_tangent and unit_normal are perpendicular
        //   implies angle = 90 degrees, thus sin(angle) = 1,
        //   thus magnitude of cross product is product of magnitudes,
        //   and each of those is known to be 1.


        for (uint32_t k = 0; k <= tube_facets; ++k)  //  HAZARD : loop upper limit is <= not < !!!!
        {
            //     Poloidal loop: 

            float t_fraction = k / (float)tube_facets;

            float angle_phi = t_fraction * 2 * XM_PI;

            float C_x = tube_radius * cosf(angle_phi);
            float C_y = tube_radius * sinf(angle_phi);

            float N_x = XMVectorGetX(unit_normal);
            float N_y = XMVectorGetY(unit_normal);
            float N_z = XMVectorGetZ(unit_normal);

            float B_x = XMVectorGetX(binormal);
            float B_y = XMVectorGetY(binormal);
            float B_z = XMVectorGetZ(binormal);

            float P_x = (curve_derivatives->at(idx_section)).axon_position_r.x + C_x * N_x + C_y * B_x;

            float P_y = (curve_derivatives->at(idx_section)).axon_position_r.y + C_x * N_y + C_y * B_y;

            float P_z = (curve_derivatives->at(idx_section)).axon_position_r.z + C_x * N_z + C_y * B_z;

            VHG_VertexPosTex tmp_surface_point = { XMFLOAT3(P_x, P_y, P_z), XMFLOAT2(0.f, 0.f) };
            
            
            //      method to retire std::vector surface_points:
            //       
            //   replace the surface_points->push_back 
            //   with 
            //          quad_top_left = { surface_points->at(0 + ii).pos, XMFLOAT2(0.f, 1.f) };
            //          quad_top_right = { surface_points->at((tube_facets + 1) + ii).pos, XMFLOAT2(0.f, 0.f) };
            //  but THIS IS IMPOSSIBLE! 
            //
            //  The expressions for quad_top_left and quad_top_right etc.. look ahead
            //  further into the evolving tube data than has yet been constructed.
            //      
            
            
            
            surface_points->push_back(tmp_surface_point);

            //  std::vector just for LINELIST topology to show radial segments
            //  from space curve out to points on the tube surface:

            VHG_VertexPosTex tmp_spoke;

            tmp_spoke.pos = (curve_derivatives->at(idx_section)).axon_position_r;
            tmp_spoke.texco = XMFLOAT2(0.f, 0.f);

            spoke_lines->push_back(tmp_spoke);
            spoke_lines->push_back(tmp_surface_point);
        }
        // for each cross-section facet;
    }

    m_vertex_buffer_1_count = spoke_lines->size();
    
    Create_Vertex_Buffer(
        spoke_lines,
        m_vertex_buffer_1_buffer.ReleaseAndGetAddressOf() 
    );  // use topology = LINELIST;
    
    //     Expected cardinality of std::vector surface_points = e_axon_arc_density * (1 + tube_facets).

    VHG_VertexPosTex quad_top_left;
    VHG_VertexPosTex quad_top_right;
    VHG_VertexPosTex quad_bottom_right;
    VHG_VertexPosTex quad_bottom_left;

    for (uint32_t i_axial = 0; i_axial < -1 + e_axon_arc_density; i_axial++)
    {
        for (uint32_t i_poloidal = 0; i_poloidal < tube_facets; i_poloidal++)
        {
            //   set the texture coordinates: 

            uint32_t ii = i_poloidal + (tube_facets + 1) * i_axial;

            quad_top_left = { surface_points->at(0 + ii).pos, XMFLOAT2(0.f, 1.f) };
            quad_top_right = { surface_points->at((tube_facets + 1) + ii).pos, XMFLOAT2(0.f, 0.f) };
            quad_bottom_right = { surface_points->at((tube_facets + 2) + ii).pos, XMFLOAT2(1.f, 0.f) };
            quad_bottom_left = { surface_points->at(1 + ii).pos, XMFLOAT2(1.f, 1.f) };

            //      Synthesize the 1st of the 2 triangles:
            surface_triangles->push_back(quad_top_left);
            surface_triangles->push_back(quad_top_right);
            surface_triangles->push_back(quad_bottom_right);

            //      Synthesize the 2nd triangle:
            surface_triangles->push_back(quad_bottom_right);
            surface_triangles->push_back(quad_bottom_left);
            surface_triangles->push_back(quad_top_left);
        }
    }


    m_vertex_buffer_2_count = surface_triangles->size(); 

    Create_Vertex_Buffer(
        surface_triangles,
        m_vertex_buffer_2_buffer.ReleaseAndGetAddressOf() 
    );  // use topology = TRIANGLELIST;

    //   TODO:   need an Index Buffer




    delete surface_triangles; surface_triangles = nullptr;

    delete spoke_lines; spoke_lines = nullptr;

    delete surface_points; surface_points = nullptr;

    delete transported_normal; transported_normal = nullptr;

    delete node_unit_tangent; node_unit_tangent = nullptr;

    delete curve_derivatives; curve_derivatives = nullptr;
}  
//  Closes VHG_Scene3D1::HansonParallelTransportFrame(); 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Load_Texture_Triax(void)
{
    Microsoft::WRL::ComPtr<ID3D11Resource>   temp_resource;

    auto d3dDevice = m_deviceResources->GetD3DDevice();

    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            d3dDevice,  //  m_d3dDevice.Get(), 
            L"Assets\\GV_Texture.png",
            temp_resource.ReleaseAndGetAddressOf(),
            e_texture_srv.ReleaseAndGetAddressOf(),
            0
        )
    );

}  
//  Closes VHG_Scene3D1::Load_Texture_Triax(); 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Create_Rasterizer_State(void)
{
    D3D11_RASTERIZER_DESC raster_desc;
    ZeroMemory(&raster_desc, sizeof(raster_desc));

    raster_desc.FillMode = this->e_rasterizer_fill_mode;
    raster_desc.CullMode = D3D11_CULL_NONE;
    raster_desc.FrontCounterClockwise = FALSE;

    raster_desc.DepthBias = 0;
    raster_desc.SlopeScaledDepthBias = 0.0f;
    raster_desc.DepthBiasClamp = 0.0f;

    raster_desc.DepthClipEnable = TRUE;
    raster_desc.ScissorEnable = FALSE;

    raster_desc.MultisampleEnable = FALSE;
    raster_desc.AntialiasedLineEnable = FALSE;


    DX::ThrowIfFailed(
        this->m_deviceResources->GetD3DDevice()->CreateRasterizerState(
            &raster_desc,
            e_rasterizer_state.ReleaseAndGetAddressOf()
        )
    );

}  
//  Closes VHG_Scene3D1::Create_Rasterizer_State; 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Create_Input_Layout(const std::vector<byte>& p_byte_vector)
{
    static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateInputLayout(
            vertexDesc,
            ARRAYSIZE(vertexDesc),
            &p_byte_vector[0], 
            p_byte_vector.size(), 
            &t_inputLayout
        )
    );

}  
// Closes VHG_Scene3D1::Create_Input_Layout(); 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::CreateDeviceDependentResources()
{ 
    //      
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //                 Load Textures from Image Files          
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //  

    this->Load_Texture_Triax();


    //      
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //                     Create a SamplerState 
    //               to be used inside the Pixel Shader           
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //  

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateSamplerState(
                &sampDesc,
                e_texture_sampler_state.ReleaseAndGetAddressOf()
        )
    );
    

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  


    Create_Rasterizer_State();


    //      
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //                Load shaders asynchronously 
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //  

    auto loadVSTask = DX::ReadDataAsync(L"ghv_PTF_VertexShader.cso");
    auto loadPSTask = DX::ReadDataAsync(L"ghv_PTF_PixelShader.cso");

    //      
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //                   Create VS Vertex Shader 
    //                   Create Input Layout Object              
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //  

    auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
    {
            DX::ThrowIfFailed(
                m_deviceResources->GetD3DDevice()->CreateVertexShader(
                    &fileData[0],
                    fileData.size(),
                    nullptr,
                    &t_vertexShader
                )
            );

            this->Create_Input_Layout(fileData);
    });


    //      
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //                        Create PS Pixel Shader 
    //                   Create the WVP Constant Buffer               
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
    //  

    auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
    {
            DX::ThrowIfFailed(
                m_deviceResources->GetD3DDevice()->CreatePixelShader(
                    &fileData[0],
                    fileData.size(),
                    nullptr,
                    &t_pixelShader )
            );

            CD3D11_BUFFER_DESC constantBufferDesc(
                sizeof(VHG_ConBuf_MVP_Struct),
                D3D11_BIND_CONSTANT_BUFFER
            );

            static_assert(
                (sizeof(VHG_ConBuf_MVP_Struct) % 16) == 0,
                "Constant Buffer struct must be 16-byte aligned"
                );

            DX::ThrowIfFailed(
                m_deviceResources->GetD3DDevice()->CreateBuffer(
                    &constantBufferDesc,
                    nullptr,
                    &m_WVP_constantBuffer )
            );
    });


    auto createCubeTask = (createPSTask && createVSTask).then([this]()
    {
            this->HansonParallelTransportFrame();
    });


    createCubeTask.then([this]()
    {
            this->m_loadingComplete = true;
    });

}  
//  Closes VHG_Scene3D1::CreateDeviceDependentResources()


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::ReleaseDeviceDependentResources()
{
    m_loadingComplete = false; 

    m_WVP_constantBuffer.Reset(); 

}  
//  Closes  VHG_Scene3D1::ReleaseDeviceDependentResources; 




//                 ...file ends... 

