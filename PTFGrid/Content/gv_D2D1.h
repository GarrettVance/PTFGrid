//                      
//        
//                           gv_D2D1.h
//
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
//  


#pragma once


#include "..\Common\DeviceResources.h"



template<class Interface>
inline void SafeRelease(
    Interface **ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}






namespace VHG
{

    class VHG_D2D1
    {
    public:

        VHG_D2D1(const std::shared_ptr<DX::DeviceResources>& deviceResources);


        VHG_D2D1::~VHG_D2D1(void); 


        void VHG_D2D1::CreateSizeDependentResources(void); 


        void VHG_D2D1::CreateDeviceResources(void); 




        void VHG_D2D1::Invalidate(void)
        {
            m_d2d1_invalid = true;
        }




        std::vector<std::wstring>       * VHG_D2D1::GetVectorWStr(void)
        {
            return m_vector_of_wstrings;
        }





        uint32_t  VHG_D2D1::Get_Schlafli_p(void)
        {
            return m_schlafli_p;
        } 




        uint32_t  VHG_D2D1::Get_Schlafli_q(void)
        {
            return m_schlafli_q;
        }




        void VHG_D2D1::basic_fit(
            double q_x0,
            double q_x1,
            double q_y0,
            double q_y1,
            double q_left,
            double q_right,
            double q_bottom,
            double q_top
        ); 




        void VHG_D2D1::fitToWindow(double w_x0, double w_x1, double w_y0, double w_y1); 


        void VHG_D2D1::draw_52_Point(double c_x, double c_y, int c_nPixels); 






        void VHG_D2D1::gv_d2d1_draw_line(
            double pt_x0, 
            double pt_y0, 
            double pt_x1, 
            double pt_y1, 
            float p_nPixels
        ); 





        void VHG_D2D1::gv_d2d1_create_geometry(void); 






        void VHG_D2D1::gv_d2d1_draw_array_points(int p_nPixels);  // keep

        void VHG_D2D1::gv_d2d1_draw_point_int(int p_x, int p_y, int p_nPixels);  

        void VHG_D2D1::gv_d2d1_draw_point_double(double p_x, double p_y, int p_nPixels); 

        void VHG_D2D1::gv_d2d1_draw_fill_int(int p_x0, int p_y0, int p_x1, int p_y1); 



        void VHG_D2D1::Render_d2d1(void); 



    private: 

        std::shared_ptr<DX::DeviceResources>        d_deviceResources;

        bool                                        m_d2d1_invalid; 

        Microsoft::WRL::ComPtr<ID2D1PathGeometry>   m_d2d1_path_geometry; 


        uint32_t                                    m_schlafli_p;
        uint32_t                                    m_schlafli_q;

        ID2D1SolidColorBrush*                       m_Brush1;

        ID2D1SolidColorBrush*                       m_Fill_Interior_Brush;
        ID2D1SolidColorBrush*                       m_Draw_Boundary_Brush;
    

        double                                      m_scaleX;
        double                                      m_scaleY;
        
        double                                      m_translateX;
        double                                      m_translateY; 


         
        std::vector<std::wstring>                  *m_vector_of_wstrings;


    };  //  Closes class VHG_D2D1; 


}  //  Closes namespace VHG; 



                                                                
