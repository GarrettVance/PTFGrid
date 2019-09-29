//                      
//             
//                     ghv : Garrett Vance : 2018_01_13
//  
//                           file  PTFGridMain.h  
//         
//         Implementation of Parallel Transport Frames for DirectX D3D11
//      
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
//  

#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\gv_SCENE_3D.h"
#include "Content\gv_HUD.h"
#include "Content\gv_D2D1.h"



namespace VHG
{
	class GVMain : public DX::IDeviceNotify
	{

	public:
		GVMain(
			const std::shared_ptr<DX::DeviceResources>& deviceResources
		);

		~GVMain();

		void CreateWindowSizeDependentResources();

		void Update();

		bool Render();


		// IDeviceNotify

		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();


        static std::unique_ptr<VHG_D2D1>       m_singleton_d2d1;



	private:

		std::shared_ptr<DX::DeviceResources>                m_deviceResources;

        std::unique_ptr<VHG_Scene3D1>                       m_sceneRenderer;

        std::unique_ptr<VHG_HUD1>                           m_overlay_upper1;  
        std::unique_ptr<VHG_HUD1>                           m_overlay_upper2;
        std::unique_ptr<VHG_HUD1>                           m_overlay_lower;

        DX::StepTimer                                       m_timer;
    };

}  //  Closes namespace VHG; 


//                  ...file ends...


