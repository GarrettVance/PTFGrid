//                      
//             
//           ghv : Garrett Vance : 2018_01_14
//  
//       Implementation of Parallel Transport Frames
//
//      DirectX 11 App (Universal Windows), Windows 10
// 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  

#include "pch.h"
#include "gv_SCENE_3D.h"
#include "..\Common\DirectXHelper.h"

using namespace VHG;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Windows::Foundation;

extern bool g0_Button_Processed; 


#ifdef GHV_OPTION_TREFOIL_KNOT
static const DirectX::XMVECTORF32 START_POSITION = { 0.0f, 0.0f, -21.f, 0.0f };
#else
//  For Lorenz Attractor:
//  static const DirectX::XMVECTORF32 START_POSITION = { 0.0f, 0.0f, -45.f, 0.0f };
//  static const DirectX::XMVECTORF32 START_POSITION = { 0.0f, 0.0f, -25.f, 0.0f };
static const DirectX::XMVECTORF32 START_POSITION = { 0.0f, 0.0f, -35.f, 0.0f };
#endif


static const float risk_speed = 40.f; 
static const float ROTATION_GAIN = 0.004f;
static const float MOVEMENT_GAIN = 0.07f;


double                   g0_total_seconds = 0.0;
double                   g0_total_sec_stop = 0.0;
double                   g0_total_sec_resume = 0.0;
double                   g0_eff_total_seconds = 0.0; 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


DirectX::SimpleMath::Vector3 gv_Calc_kminput_x_y_z_From_Pitch_Yaw(float p_pitch, float p_yaw)
{
    float   kminput_f_y = sinf(p_pitch);
    float   kminput_f_r = cosf(p_pitch);

    float   kminput_f_z = kminput_f_r * cosf(p_yaw);
    float   kminput_f_x = kminput_f_r * sinf(p_yaw);

    DirectX::SimpleMath::Vector3 kmi_input_as_vector = DirectX::SimpleMath::Vector3(
        kminput_f_x, kminput_f_y, kminput_f_z);

    return kmi_input_as_vector;
}  
//  Closes gv_Calc_kminput_x_y_z_From_Pitch_Yaw; 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::GV_Camera_Init(void)
{
    //     Update the View Matrix and Camera Position

    e_camera_position = START_POSITION.v;

    DirectX::SimpleMath::Vector3 tmp_kminput = gv_Calc_kminput_x_y_z_From_Pitch_Yaw(e_pitch, e_yaw);

    DirectX::SimpleMath::Vector3 camera_target = e_camera_position + tmp_kminput;

    DirectX::SimpleMath::Vector3 camera_up_vector = DirectX::SimpleMath::Vector3::Up;

    this->e_view_matrix = XMMatrixLookAtLH(e_camera_position, camera_target, camera_up_vector);

    //  
    //    matrices are stored in their TRANSPOSE form. 
    // 

    XMStoreFloat4x4(&m_WVP_constant_buffer_data.view, XMMatrixTranspose(e_view_matrix));

}  
//  Closes  VHG_Scene3D1:GV_Camera_Init;  


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::animator_update(void)
{
    static uint32_t   gv_loop_count = 0;

    if (gv_loop_count > 10000)
    {
        gv_loop_count = 0;
    }

    gv_loop_count++;


    uint32_t mod_anim_old = 0;
    uint32_t mod_anim_new = 0;

    //  gv_loop_count modulo "speed" around the attractor; 
    //  try e.g. speed of 10. 

    uint32 speed_of_cube_animation = 8;  // GOLD: 10; 

    if (gv_loop_count % speed_of_cube_animation == 0)
    {
        mod_anim_old = m_WVP_constant_buffer_data.animator_count.x;

        mod_anim_new = (1 + mod_anim_old) % (1 + 2 * e_advert_modulus);

        this->m_WVP_constant_buffer_data.animator_count = XMUINT4(mod_anim_new, 0, 0, 0);
    }
}  
//  Closes VHG_Scene3D1::animator_update; 



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void VHG_Scene3D1::do_rotation_y_axis(DirectX::XMMATRIX&  p_rot_mat)
{
    double total_sec = 0.f;

    if (e_option_rotate_lorenz)
    {
        total_sec = g0_total_seconds - (g0_total_sec_resume - g0_total_sec_stop);  // perfect!!!

        g0_eff_total_seconds = total_sec;
    }
    else
    {
        total_sec = g0_total_sec_stop;
    }

    double totalRotation = total_sec * XMConvertToRadians(m_degreesPerSecond);
    float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

    if (e_option_flip_x)
    {
        //   The spiral's axis is vertical:
        // 
        //   This case will compose two rotations:
        //   Rotate pi/2 radians about the x-axis...
        //   Rotate "radians" radians about the y-axis...
        //    

        p_rot_mat = XMMatrixRotationX(DirectX::XM_PI / 2.f) * XMMatrixRotationY(radians);

    }
    else
    {
        //    Looking into the corkscrew straight-on: 
        // 
        //    Rotate about the z-axis!!!

        p_rot_mat = XMMatrixRotationZ(radians);
    }
}  
//  Closes VHG_Scene3D1::do_rotation_y_axis; 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::gv_next_mesh_object_shape(void)
{
    std::vector<MeshObjectShape>      gvect_shapes; 

    gvect_shapes.push_back(MeshObjectShape::Lissajous); 
    gvect_shapes.push_back(MeshObjectShape::Spiral); 
    gvect_shapes.push_back(MeshObjectShape::ParabolicHelix); 
    gvect_shapes.push_back(MeshObjectShape::Corkscrew); 

    MeshObjectShape  tmp1;

    for (std::vector<MeshObjectShape>::iterator it = gvect_shapes.begin();
        it != gvect_shapes.end(); ++it)
    {
        tmp1 = *it; 

        if (*it == this->e_mesh_object_shape)
        {
            if (e_mesh_object_shape == gvect_shapes.back())
            {
                e_mesh_object_shape = gvect_shapes.front();
            }
            else
            {
                e_mesh_object_shape = *(1 + it);
            }

            break; 
        }
    }
}
//  Closes VHG_Scene3D1::gv_next_mesh_object_shape();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void VHG_Scene3D1::Update(DX::StepTimer const& timer)
{
    //     calculates the model and view matrices once per frame.

    float            gv_elapsed_time = float(timer.GetElapsedSeconds());

    //  
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                        Mouse Handling: 
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //   
    //      
    DirectX::Mouse::State    maus_state = e_mouse->GetState();

    if (maus_state.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta_mouse = ROTATION_GAIN * Vector3(float(maus_state.x), float(maus_state.y), 0.f);

        e_pitch -= delta_mouse.y;

        e_yaw -= delta_mouse.x;



        //    limit pitch to straight up or straight down
        //    with a small deviation to avoid gimbal lock  

#undef max
#undef min

        float gimbal_limit = XM_PI / 2.0f - 0.01f;
        e_pitch = std::max(-gimbal_limit, e_pitch);
        e_pitch = std::min(+gimbal_limit, e_pitch);

        // keep longitude within a sane range by wrapping  

        if (e_yaw > XM_PI)
        {
            e_yaw -= XM_PI * 2.0f;
        }
        else if (e_yaw < -XM_PI)
        {
            e_yaw += XM_PI * 2.0f;
        }


    }  //  Closes "if" the mouse mode is relative; 


    e_mouse->SetMode(maus_state.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);


    //  
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //   
    //                        Keyboard Handling: 
    //   
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //   
    //      
    DirectX::Keyboard::State           kb = e_keyboard->GetState();

    DirectX::SimpleMath::Vector3       kminput_move = DirectX::SimpleMath::Vector3::Zero;

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.Escape)
    {
        //   When using MODE_RELATIVE, the system cursor is hidden 
        //   so a user can't navigate away to another monitor or app or even exit. 
        //
        //   If your game makes use of 'mouse-look' controls, 
        //   you should ensure that a simple key (like the ESC key) 
        //   returns to the game's menu/pause screen 
        //   
        //   Moreover, that key press to pause must restore MODE_ABSOLUTE behavior.
        // 
        //   https://github.com/Microsoft/DirectXTK/wiki/Mouse 
        //   


        // ghv : todo :  assign some key to restore absolute behavior:    m_mouse->SetMode(Mouse::MODE_ABSOLUTE);


        Windows::ApplicationModel::Core::CoreApplication::Exit(); 

    }  //  Closes "Escape"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.P)
    {
        auto av = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

        if (!av->IsFullScreenMode)
        {
            if (av->TryEnterFullScreenMode())
            {
                av->FullScreenSystemOverlayMode = Windows::UI::ViewManagement::FullScreenSystemOverlayMode::Minimal;
            }
        }
    }  //  Closes "P"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.L)
    {
        auto av = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

        if (av->IsFullScreenMode) 
        {
            av->ExitFullScreenMode();
            av->FullScreenSystemOverlayMode = Windows::UI::ViewManagement::FullScreenSystemOverlayMode::Standard;
        }
    }  //  Closes "L"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.O)
    {       
        //   A request to resume the rotation: 

        if (!e_option_rotate_lorenz)
        {   
            g0_total_sec_resume = g0_total_seconds;

            e_option_rotate_lorenz = true; 
        }
    }  //  Closes "O"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.K)
    {
        //    Request to stop the rotation. 

        if (e_option_rotate_lorenz)
        {
            g0_total_sec_stop = g0_eff_total_seconds;   

            e_option_rotate_lorenz = false;
        }
    }  //  Closes "K"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.F3)
    {
        //   A request to show wireframe: 

        if (e_rasterizer_fill_mode == D3D11_FILL_SOLID)
        {
            e_rasterizer_fill_mode = D3D11_FILL_WIREFRAME;
        }
    }  //  Closes "F3"; 


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.F4)
    {
        //   A request to de-activate wireframe and return to "solid" rendering: 

        if (e_rasterizer_fill_mode == D3D11_FILL_WIREFRAME)
        {
            e_rasterizer_fill_mode = D3D11_FILL_SOLID;
        }
    }  //  Closes "F4"; 

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.F11)
    {
        //   A request to change MeshObjectShape: 

        if (!g0_Button_Processed)
        {
            g0_Button_Processed = true;

            gv_next_mesh_object_shape(); 

            this->HansonParallelTransportFrame();
        }
    }  //  Closes "F11"; 

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.F12)
    {
        //   A request to toggle the pi/2 rotation about x-axis:

        if (!g0_Button_Processed)
        {
            g0_Button_Processed = true; 

            if (e_option_flip_x)
            {
                e_option_flip_x = false;
            }
            else
            {
                e_option_flip_x = true;
            }

            this->HansonParallelTransportFrame();
        }
    }  //  Closes "F12"; 

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.M)
    {
        //   A request to increment or decrement angular_m: 

        if (!g0_Button_Processed)
        {
            g0_Button_Processed = true; 

            if (kb.LeftShift || kb.RightShift || kb.CapsLock)
            {
                if (e_angular_m < 6)
                {
                    e_angular_m += 1;
                }
                else
                {
                    e_angular_m = 1;
                }
            }
            else
            {
                if (e_angular_m > 1)
                {
                    e_angular_m -= 1;
                }
                else
                {
                    e_angular_m = 6;
                }
            }

            this->HansonParallelTransportFrame();
        }
    }  //  Closes "letter M"; 

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.N)
    {
        //   A request to increment or decrement angular_n: 

        if (!g0_Button_Processed)
        {
            g0_Button_Processed = true; 

            if (kb.LeftShift || kb.RightShift || kb.CapsLock)
            {
                if (e_angular_n < 6)
                {
                    e_angular_n += 1;
                }
                else
                {
                    e_angular_n = 1;
                }
            }
            else
            {
                if (e_angular_n > 1)
                {
                    e_angular_n -= 1;
                }
                else
                {
                    e_angular_n = 6;
                }
            }

            this->HansonParallelTransportFrame();
        }
    }  //  Closes "letter N"; 

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.Home)
    {
        e_camera_position = START_POSITION.v;
        e_pitch = e_yaw = 0;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.Up || kb.W)
    {
        kminput_move.y += risk_speed * gv_elapsed_time;  
    }

    if (kb.Down || kb.S)
    {
        kminput_move.y -= risk_speed * gv_elapsed_time;   
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.Left || kb.A)
    {
        kminput_move.x -= risk_speed * gv_elapsed_time;
    }

    if (kb.Right || kb.D)
    {
        kminput_move.x  += risk_speed * gv_elapsed_time;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (kb.PageUp || kb.Space)
    {
        kminput_move.z += risk_speed * gv_elapsed_time; 
    }

    if (kb.PageDown || kb.X)
    {
        kminput_move.z -= risk_speed * gv_elapsed_time;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 
    //                Free-Look Camera 
    // 



    DirectX::SimpleMath::Vector3 risk_DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.f);
    DirectX::SimpleMath::Vector3 risk_DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.f);

    // DirectX::SimpleMath::Vector3 risk_DefaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);




    DirectX::SimpleMath::Vector3 risk_camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.f);
    DirectX::SimpleMath::Vector3 risk_camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.f);

    //  see below  DirectX::SimpleMath::Vector3 risk_camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);





    DirectX::SimpleMath::Matrix   risk_camRotationMatrix = XMMatrixRotationRollPitchYaw(e_pitch, e_yaw, 0);

    risk_camRight = XMVector3TransformCoord(risk_DefaultRight, risk_camRotationMatrix);
    risk_camForward = XMVector3TransformCoord(risk_DefaultForward, risk_camRotationMatrix); 


    DirectX::SimpleMath::Vector3 risk_camUp = XMVector3Cross(risk_camForward, risk_camRight);



    kminput_move *= MOVEMENT_GAIN;


    e_camera_position += kminput_move.x * risk_camRight; 
    e_camera_position += kminput_move.z * risk_camForward;


    kminput_move = DirectX::SimpleMath::Vector3::Zero;  //  redundant ;



    DirectX::SimpleMath::Vector3  risk_camTarget = XMVector3TransformCoord(
        risk_DefaultForward, 
        risk_camRotationMatrix); 

    risk_camTarget = XMVector3Normalize(risk_camTarget); 

    risk_camTarget += e_camera_position;


    //  
    //       Update the View Matrix 
    //  

    this->e_view_matrix = XMMatrixLookAtLH(e_camera_position, risk_camTarget, risk_camUp);


    //  
    //       This application stores WVP matrices in their TRANSPOSE form. 
    //  

    XMStoreFloat4x4(
        &m_WVP_constant_buffer_data.view, 
        XMMatrixTranspose(
            e_view_matrix
        )
    );



    this->animator_update(); 
    



    g0_total_seconds = timer.GetTotalSeconds();

    XMMATRIX lorenz_rotation;  
    do_rotation_y_axis(lorenz_rotation);

#ifdef GHV_OPTION_TREFOIL_KNOT
    //  for Trefoil Knot:  
    XMMATRIX lorenz_scaling = XMMatrixScaling(4.f, 4.f, 4.f); 
#else 
    // XMMATRIX lorenz_scaling = XMMatrixScaling(0.6f, 0.6f, 0.6f); 
    XMMATRIX lorenz_scaling = XMMatrixScaling(1.f, 1.f, 1.f); 
#endif


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //          USE RST:  
    //    world matrix = rotation_mx * scaling_mx * translation_mx; 
    // 
    //
    //         world translation matrix 
    //         ========================
    // 
    //         Generally, if the CAMERA_START_POSITION 
    //         has x = 0 and y = 0, 
    //         then no world translation is required...
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++

    XMMATRIX lorenz_world_matrix = lorenz_rotation * lorenz_scaling;  

#ifdef GHV_OPTION_TREFOIL_KNOT
    ;
#else


    if (e_option_flip_x)
    {
        // So-so for Lor.Attractor:   lorenz_translation = (0.f, 20.f, 20.f);
        // Very good for Lor.Attr.:   lorenz_translation = (0.f, 20.f, 30.f);

        XMMATRIX lorenz_translation = XMMatrixTranslation(0.f, 20.f, 30.f);


        //   Use world_matrix = translation * world_matrix: 

        lorenz_world_matrix = lorenz_world_matrix * lorenz_translation;
    }


#endif


        
    XMStoreFloat4x4(&m_WVP_constant_buffer_data.model, 
        XMMatrixTranspose(
                lorenz_world_matrix
        )
    );


    //   Evolution of the tube extrusion

    static double prior_total_seconds = 0;
    static int direction_factor = 1;
    
    // const double time_delay = 0.4;
    // const double time_delay = 0.07;  
    const double time_delay = 0.02;

#ifdef GHV_OPTION_ANIMATE_CURVE_EVOLUTION

    if (g0_total_seconds - prior_total_seconds > time_delay)
    {
        prior_total_seconds = g0_total_seconds;

        const UINT tube_quantum = (3 + 3) * tube_facets;


        //  Note that e_axon_arc_density is aka tube_axial_steps; 

        const UINT half_of_axial_steps = -3 + (UINT)floor(e_axon_arc_density / 2.f);

        const UINT count_maximum = tube_quantum * half_of_axial_steps;

        const UINT count_minimum = tube_quantum * 2;

        if(m_vtxbuf_2_draw_count > count_maximum)
        {
            direction_factor = -1;
        }
        else if(m_vtxbuf_2_draw_count < count_minimum)
        {
            direction_factor = 1;
        }

        m_vtxbuf_2_draw_count += direction_factor * tube_quantum;
    }
#else 
    m_vtxbuf_2_draw_count = m_vertex_buffer_2_count;
#endif

}  
//  Closes  VHG_Scene3D1::Update;  


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//                 ...file ends... 

