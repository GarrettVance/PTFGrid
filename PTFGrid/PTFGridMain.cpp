//                      
//                      
//                      
//                     ghv : Garrett Vance : 2018_01_13
//                      
//                           file  PTFGridMain.cpp
//                      
//    Implementation of Parallel Transport Frames for DirectX D3D11
//    following Andrew J. Hanson's paper
//    "Parallel Transport Approach to Curve Framing" 
//    of January 1995.
//                      
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                      



#include "pch.h"
#include "PTFGridMain.h"
#include "Common\DirectXHelper.h"

using namespace VHG;

using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;



//   ghv : a static data member must be 
//         defined OUTSIDE of class scope: 

std::unique_ptr<VHG_D2D1>         GVMain::m_singleton_d2d1;








GVMain::GVMain(
    const std::shared_ptr<DX::DeviceResources>& formal_deviceResources
) : m_deviceResources(formal_deviceResources)
{
	//     Class ctor for GVMain class 



	// Register to be notified if the Device is lost or recreated

	m_deviceResources->RegisterDeviceNotify(this);


	m_sceneRenderer = std::unique_ptr<VHG_Scene3D1>(
                                 new VHG_Scene3D1(m_deviceResources));


    GVMain::m_singleton_d2d1 = std::unique_ptr<VHG_D2D1>(
                                new VHG_D2D1(formal_deviceResources));




	m_overlay_upper1 = std::unique_ptr<VHG_HUD1>(new VHG_HUD1(
			m_deviceResources,
			0.05f,    //  upper 
			0.05f,   //  LEFT !!! 
			D2D1::ColorF(D2D1::ColorF::Violet)));

	m_overlay_upper2 = std::unique_ptr<VHG_HUD1>(new VHG_HUD1(
			m_deviceResources,
			0.05f,    //  upper 
			0.999f,   //  right 
			D2D1::ColorF(D2D1::ColorF::Yellow)));

	m_overlay_lower = std::unique_ptr<VHG_HUD1>(new VHG_HUD1(
			m_deviceResources,
			0.999f,    //  lower  
			0.999f,    //  right  
			D2D1::ColorF(D2D1::ColorF::Red)));


    GVMain::m_singleton_d2d1->CreateDeviceResources();

    GVMain::m_singleton_d2d1->Invalidate();
    GVMain::m_singleton_d2d1->CreateSizeDependentResources();


}  
//  Closes class ctor;
















GVMain::~GVMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}







void GVMain::CreateWindowSizeDependentResources() 
{
    //      Updates application state 
    //      when the window size changes 
    //      (e.g. device orientation change)
   
	m_sceneRenderer->CreateWindowSizeDependentResources();


    GVMain::m_singleton_d2d1->Invalidate();
    GVMain::m_singleton_d2d1->CreateSizeDependentResources();


}  
//  Closes GVMain::CreateWindowSizeDependentResources(); 










void GVMain::Update() 
{
	//      Update the scene objects once per frame:


	m_timer.Tick([&]()
	{
		m_sceneRenderer->Update(m_timer);  

		{
            /*
			static  wchar_t   buffer1[64];
			const wchar_t  fmt1[] = L"Frequency Ratio: %d  / %d"; 
			std::swprintf(
                buffer1, 
                63, 
                fmt1, 
                m_sceneRenderer->get_angular_m(), 
                m_sceneRenderer->get_angular_n()
            ); 
			std::wstring wstr_upper1(buffer1);
            */

			std::wstring wstr_upper1(L"Lorenz Attractor");

			m_overlay_upper1->Update(m_timer, wstr_upper1);
		}



		{
			uint32_t triangles_per_render = m_sceneRenderer->Get_Vertex_Count_Triax() / 3; 



            uint32 v_fps = m_timer.GetFramesPerSecond();
            uint32_t  triangles_per_second = max(v_fps, 1) * triangles_per_render; 

            wchar_t   buffer2[24];
            const wchar_t  fmt2[] = L"%d\n"; 
			std::swprintf(buffer2, 17, fmt2, triangles_per_second); 


            int num_digits = -1 + wcslen(buffer2); 
            std::wstring wstr_int_with_commas(L"");
            int idx_comma_string = 0; 
            int distance_comma_string = 0;

            for (idx_comma_string = 0; buffer2[idx_comma_string] != '\0'; idx_comma_string++)
            {
                wstr_int_with_commas += buffer2[idx_comma_string]; 

                distance_comma_string = num_digits - idx_comma_string - 1; 

                if (distance_comma_string > 0 && distance_comma_string % 3 == 0) wstr_int_with_commas += ',';
            }

            std::wstring wstr_upper2(L"Triangles per Second: "); 
            wstr_upper2 += wstr_int_with_commas; 
			m_overlay_upper2->Update(m_timer, wstr_upper2);
		}


		{
			uint32_t animation_index = m_sceneRenderer->gv_GetAnimatorCount(); 

			if (animation_index > m_sceneRenderer->gv_get_advert_modulus())
			{
				std::wstring wstr_lower = L"The Fractal Method";
				m_overlay_lower->Update(m_timer, wstr_lower);
			}
			else
			{
				uint32 v_fps = m_timer.GetFramesPerSecond();
				std::wstring wstr_lower = (v_fps > 0) ? std::to_wstring(v_fps) + L" Frames per second" : L" - FPS";
				m_overlay_lower->Update(m_timer, wstr_lower);
			}
		}
	});

}  
//  Closes GVMain::Update; 






bool GVMain::Render() 
{
	// Don't try to render anything before the first Update.

	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();


	// Reset the viewport to target the whole screen.

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);



	// Reset render targets to the screen.

	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());



	// Clear the back buffer and depth stencil view.  




	context->ClearRenderTargetView(
		m_deviceResources->GetBackBufferRenderTargetView(),
		DirectX::Colors::Black  // GHV_COLOR_OF_WORLD
	);




	context->ClearDepthStencilView(
		m_deviceResources->GetDepthStencilView(), 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
		1.0f, 
		0);




	// Render the scene objects.

	m_sceneRenderer->Render();


    GVMain::m_singleton_d2d1->Render_d2d1(); // Hyperbolic Geometry; 


	m_overlay_upper1->Render();
	m_overlay_upper2->Render();
	m_overlay_lower->Render();

	return true; 

}  
//  Closes  GVMain::Render;  












// Notifies renderers that device resources need to be released.

void GVMain::OnDeviceLost()
{

	m_sceneRenderer->ReleaseDeviceDependentResources(); 

	m_overlay_upper1->ReleaseDeviceDependentResources();
	m_overlay_upper2->ReleaseDeviceDependentResources();
	m_overlay_lower->ReleaseDeviceDependentResources(); 

}








void GVMain::OnDeviceRestored()
{ 
    // Notifies renderers that device resources may now be recreated.
	
    
    m_sceneRenderer->CreateDeviceDependentResources();

    GVMain::m_singleton_d2d1->CreateDeviceResources();

	m_overlay_upper1->CreateDeviceDependentResources();
	m_overlay_upper2->CreateDeviceDependentResources();
	m_overlay_lower->CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();

}




//                   ...file ends....


