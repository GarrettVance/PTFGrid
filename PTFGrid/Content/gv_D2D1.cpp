//                        
//                       
//                
//                              


#include "pch.h"


#include <D2d1helper.h>
#include "..\Common\DirectXHelper.h"   //  for DX::ThrowIfFailed(); 


#include "gv_D2D1.h"




using namespace VHG;    



using Microsoft::WRL::ComPtr;


#undef  GHV_WM_SIZING_SQUARE




const D2D1_POINT_2F   gv_data_arr[21]
{
    D2D1::Point2F(-1.00f, 	0.00f),
    D2D1::Point2F(-0.80f, 	0.00f),
    D2D1::Point2F(-0.60f, 	0.00f),
    D2D1::Point2F(-0.40f, 	0.00f),
    D2D1::Point2F(-0.20f, 	0.00f),

    D2D1::Point2F(0.00f, 	0.00f),  //  The origin, center of coordinate axes;

    D2D1::Point2F(0.20f, 	0.00f),
    D2D1::Point2F(0.40f, 	0.00f),
    D2D1::Point2F(0.60f, 	0.00f),
    D2D1::Point2F(0.80f, 	0.00f),
    D2D1::Point2F(1.00f, 	0.00f),


    D2D1::Point2F(0.00f,  -1.00f),
    D2D1::Point2F(0.00f,  -0.80f),
    D2D1::Point2F(0.00f,  -0.60f),
    D2D1::Point2F(0.00f,  -0.40f),
    D2D1::Point2F(0.00f,  -0.20f),

    //   Don't plot the origin twice:   D2D1::Point2F( 0.00f,   0.00f 	), 

    D2D1::Point2F(0.00f,   0.20f),
    D2D1::Point2F(0.00f,   0.40f),
    D2D1::Point2F(0.00f,   0.60f),
    D2D1::Point2F(0.00f,   0.80f),
    D2D1::Point2F(0.00f,   1.00f),
};









const D2D1_POINT_2F gv_closed_shape_array[5] = 
{
    D2D1::Point2F(267, 177),
    D2D1::Point2F(236, 192),
    D2D1::Point2F(212, 160),
    D2D1::Point2F(156, 255),
    D2D1::Point2F(346, 255),  //  End point coincindes with Start point; 
};















VHG_D2D1::VHG_D2D1(
    const std::shared_ptr<DX::DeviceResources>& q_deviceResources
) 
    : d_deviceResources( q_deviceResources ), 
    m_d2d1_invalid(true),

    m_schlafli_p(3), 
    m_schlafli_q(3) 
{

        
    m_vector_of_wstrings = new std::vector<std::wstring>(); 



    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                         
    //                         
    //        Class constructor for hyperbolic geometry: 
    //                         
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


}  //  Closes class ctor VHG_D2D1::VHG_D2D1(); 










VHG_D2D1::~VHG_D2D1(void)
{
    if (m_vector_of_wstrings)
    {
        delete m_vector_of_wstrings; 

        m_vector_of_wstrings = nullptr; 
    }

}  //  Class destructor; 







void VHG_D2D1::CreateSizeDependentResources(void)
{
    //     Invoked whenever the window size changes.

    // fitToWindow(-1.0, +1.0, -1.0, +1.0);

}  //  Closes VHG_D2D1::CreateSizeDependentResources(); 






void VHG_D2D1::CreateDeviceResources(void)
{

    this->gv_d2d1_create_geometry(); 



    ID2D1DeviceContext2   *d2d1_ctxt = 
        this->d_deviceResources->GetD2DDeviceContext();



    DX::ThrowIfFailed(d2d1_ctxt->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::BlueViolet),
        &m_Brush1)
    ); 


    
    DX::ThrowIfFailed(d2d1_ctxt->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkGreen),
        &m_Fill_Interior_Brush)
    );



    DX::ThrowIfFailed(d2d1_ctxt->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightBlue),
        &m_Draw_Boundary_Brush)
    );


    return; 

}  //  Closes VHG_D2D1::CreateDeviceResources(); 











void VHG_D2D1::gv_d2d1_create_geometry(void)
{
    ID2D1Factory3*     gv_d2d1_factory3 = this->d_deviceResources->GetD2DFactory(); 
       
    DX::ThrowIfFailed(
        gv_d2d1_factory3->CreatePathGeometry(
            m_d2d1_path_geometry.GetAddressOf()
        )
    );
    

    ID2D1GeometrySink *pSink = NULL; 


    DX::ThrowIfFailed(
        m_d2d1_path_geometry->Open(&pSink)
    );


    pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

    uint32_t  data_cardinality = ARRAYSIZE(gv_closed_shape_array); 


    pSink->BeginFigure(
        gv_closed_shape_array[-1 + data_cardinality], //  End point coincindes with Start point; 
        D2D1_FIGURE_BEGIN_FILLED
    ); 

   
    //  pSink->AddArc(),  AddBezier, ....


    pSink->AddLines(
        gv_closed_shape_array, 
        data_cardinality
    );  //  Scan the array elements;


    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

   
    DX::ThrowIfFailed(
        pSink->Close()
    );

    SafeRelease(&pSink); 

}  //  Closes  VHG_D2D1::gv_d2d1_create_geometry(); 











void VHG_D2D1::gv_d2d1_draw_array_points(int p_nPixels)
{
    //    These subordinate drawing methods 
    //    DON'T DON'T DON'T issue BeginDraw() 
    //    nor do they issue EndDraw() calls. 


    m_Brush1->SetColor(D2D1::ColorF(D2D1::ColorF::GreenYellow));


    ID2D1DeviceContext2   *gv_d2d1_context = this->d_deviceResources->GetD2DDeviceContext();


    gv_d2d1_context->DrawLine(
        D2D1::Point2F(-0.90f, 0.00f),
        D2D1::Point2F(+0.90f, 0.00f),
        m_Brush1,
        1.f /* line width */
    );

    

}  //  Closes VHG_D2D1::gv_d2d1_draw_array_points(); 







void VHG_D2D1::basic_fit(
    double q_x0,
    double q_x1,
    double q_y0,
    double q_y1,
    double q_left,
    double q_right,
    double q_bottom,
    double q_top
)
{
    //
    //          Set translate and scale
    //          so that x0,x1,y0,y0
    //          map to left,right,bottom,top.
    //

    m_scaleX = (q_right - q_left) / (q_x1 - q_x0);

    m_scaleY = (q_top - q_bottom) / (q_y1 - q_y0);

    m_translateX = q_left - m_scaleX * q_x0;

    m_translateY = q_bottom - m_scaleY * q_y0;

}  //  Closes VHG_D2D1::basic_fit;








void VHG_D2D1::fitToWindow(double w_x0, double w_x1, double w_y0, double w_y1)
{ 
    //              
    //     The 4 formal arguments w_x0, w_x1, w_y0 and w_y1 
    //     serve to specify the horizontal and vertical lengths
    //     or bounds of the unit disk. 
    //             





    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                                   
    //                                   
    //         Major Change from D2D1One to HyperGeodesic:
    //         ===========================================
    //                                   
    //         In project D2D1One, I have a class data 
    //         member "m_pRenderTarget" which is 
    //         of type ID2D1HwndRenderTarget* . 
    //                                   
    //                                   
    //         But here in project HyperGeodesic, 
    //         I don't exactly have an ID2D1HwndRenderTarget.
    //         Rather, the Direct2D drawing is done 
    //         through a "linked target bitmap". 
    //                                   
    //                                   
    //         Consequently, I need to find a replacement 
    //         for D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();  
    //                                   
    //         Let's try using m_deviceResources->GetLogicalSize(); 
    //                                   
    // 
    //  HOWEVER: 
    //  
    //   Looking at  VHG_D2D1::CreateWindowSizeDependentResources() 
    //   near the top of that procedure, 
    //   there is a call to m_deviceResources->GetOutputSize(); 
    // 
    //   Which makes me question the wisdom of my having 
    //   used GetLogicalSize().... 
    // 
    //
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      
    // Windows::Foundation::Size   trial_size = m_deviceResources->GetLogicalSize(); 


    Windows::Foundation::Size   trial_size = this->d_deviceResources->GetOutputSize();

    D2D1_SIZE_F rtSize; 
    
    rtSize.width = trial_size.Width; 

    rtSize.height = trial_size.Height; 


    
#ifdef GHV_WM_SIZING_SQUARE

    //                 
    //        The WM_SIZING message is being used 
    //        to constrain the drawing to remain square.
    //        So don't do anything special here...
    //                     

    this->basic_fit(
        w_x0, 
        w_x1, 
        w_y0, 
        w_y1, 
        0.5,
        rtSize.width - 0.5, 
        rtSize.height - 0.5, 
        0.5
    );

#else

    //     
    //     Constrain the drawing to remain square
    //     by changing params sent to basic_fit method:
    //  

    float render_target_square = min(rtSize.width, rtSize.height); 

    this->basic_fit(
        w_x0, 
        w_x1, 
        w_y0, 
        w_y1, 
        0.5,
        render_target_square - 0.5, 
        render_target_square - 0.5, 
        0.5
    );

#endif


}  //  Closes VHG_D2D1::fitToWindow(); 





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



















void VHG_D2D1::gv_d2d1_draw_point_double(double p_x, double p_y, int p_nPixels)
{
    //    These subordinate drawing methods 
    //    DON'T DON'T DON'T issue BeginDraw() 
    //    nor do they issue EndDraw() calls. 

   
    double x0 = p_x - (0.5 * p_nPixels); 
    double y0 = p_y - (0.5 * p_nPixels);

    double minor_adjust_x0 = 1.000; 

    D2D1_RECT_F gv_drawing_rect = D2D1::RectF(
        (float)(x0 + minor_adjust_x0),
        (float)(y0),
        (float)(x0 + p_nPixels),
        (float)(y0 + p_nPixels)
    );





    ID2D1DeviceContext2   *gv_d2d1_context = this->d_deviceResources->GetD2DDeviceContext();
    gv_d2d1_context->FillRectangle(gv_drawing_rect, m_Brush1); 

}  //  Closes VHG_D2D1::gv_d2d1_draw_point_double; 








void VHG_D2D1::gv_d2d1_draw_line(
    double pt_x0, 
    double pt_y0, 
    double pt_x1, 
    double pt_y1, 
    float p_nPixels
)
{
    //    These subordinate drawing methods 
    //    DON'T DON'T DON'T issue BeginDraw() 
    //    nor do they issue EndDraw() calls.       
    
    D2D1_POINT_2F  point_begin = D2D1::Point2F(
        (float)(pt_x0 * m_scaleX + m_translateX),
        (float)(pt_y0 * m_scaleY + m_translateY)
    ); 

    D2D1_POINT_2F  point_end = D2D1::Point2F(
        (float)(pt_x1 * m_scaleX + m_translateX),
        (float)(pt_y1 * m_scaleY + m_translateY)
    ); 

    ID2D1DeviceContext2   *gv_d2d1_context = this->d_deviceResources->GetD2DDeviceContext();


    gv_d2d1_context->DrawLine(
        point_begin, 
        point_end,
        m_Brush1,
        p_nPixels,     //   path thickness; 
        NULL       //   line drawing StrokeStyle; 
    );

}  //  Closes VHG_D2D1::gv_d2d1_draw_line; 








void VHG_D2D1::Render_d2d1(void)
{
    if (!m_d2d1_invalid)
    {
        return;
    }

    ID2D1DeviceContext2       *gv_d2d1_context 
        = this->d_deviceResources->GetD2DDeviceContext();

    
    gv_d2d1_context->BeginDraw();

    gv_d2d1_context->SetTransform(D2D1::Matrix3x2F::Identity());
    
    m_Brush1->SetColor(D2D1::ColorF(D2D1::ColorF::GreenYellow));

   
    Windows::Foundation::Size   trial_size 
        = this->d_deviceResources->GetOutputSize();
  

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //          Draw a grid background
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    int rt_width = static_cast<int>(trial_size.Width);
    int rt_height = static_cast<int>(trial_size.Height);

    int delta_x = static_cast<int>(trial_size.Width / 10.f);
    int delta_y = static_cast<int>(trial_size.Height / 10.f);

    for (int x = 0; x < rt_width; x += delta_x)
    {
        gv_d2d1_context->DrawLine(
            D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
            D2D1::Point2F(static_cast<FLOAT>(x), trial_size.Height),
            m_Brush1, 0.5f );
    }

    for (int y = 0; y < rt_height; y += delta_y)
    {
        gv_d2d1_context->DrawLine(
            D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
            D2D1::Point2F(trial_size.Width, static_cast<FLOAT>(y)),
            m_Brush1, 0.5f );
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    float translateX = trial_size.Width / (+1.f - -1.f);   // typically 600.f;
    float translateY = trial_size.Height / 2.f;   // typically 450.f; 

    float scaleX = trial_size.Width / 2.f;
    float scaleY = trial_size.Height / 2.f;

    float pixel_line_width = 1;  // Probably bold enough...


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    float x_left_end = -0.95f * scaleX + translateX;  //  -(0.5f * pixel_line_width);
    float y_left_end = 0.00f * scaleY + translateY;   //   -(0.5f * pixel_line_width);

    float x_right_end = +0.95f * scaleX + translateX;  //   -(0.5f * pixel_line_width);
    float y_right_end = 0.00f * scaleY + translateY;   //   -(0.5f * pixel_line_width);

    gv_d2d1_context->DrawLine(
        D2D1::Point2F(x_left_end, y_left_end),
        D2D1::Point2F(x_right_end, y_right_end),
        m_Brush1,
        pixel_line_width /* line width */
    );

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    float x_upper_end =  0.f * scaleX + translateX;  //  -(0.5f * pixel_line_width);
    float y_upper_end = -0.95f * scaleY + translateY;   //   -(0.5f * pixel_line_width);

    float x_lower_end =  0.f * scaleX + translateX;  //   -(0.5f * pixel_line_width);
    float y_lower_end = +0.95f * scaleY + translateY;   //   -(0.5f * pixel_line_width);

    gv_d2d1_context->DrawLine(
        D2D1::Point2F(x_upper_end, y_upper_end),
        D2D1::Point2F(x_lower_end, y_lower_end),
        m_Brush1,
        pixel_line_width /* line width */
    );

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    
/*********************************************************************************    
    //         
    //      ghv : for the D2D1 Transforms (translate and scale)
    //            the grid "domain" is assumed to be [-1.0, +1.0]
    //            in both the x- and y-directions...

    
    float translateX = trial_size.Width / (+1.f - -1.f);   // typically 600.f;
    float translateY = trial_size.Height / 2.f;   // typically 450.f; 
    const D2D1::Matrix3x2F  translate_matrix = D2D1::Matrix3x2F::Translation(translateX, translateY);
    float scaleX = 600.0f; //  trial_size.Width / 2.f;
    float scaleY = 5.0f; // trial_size.Height / 2.f;
    D2D1_SIZE_F gv_scale_factor = { scaleX, scaleY };
    D2D1_POINT_2F gv_center = { 0.f, 0.f }; 
    const D2D1::Matrix3x2F scale_matrix = D2D1::Matrix3x2F::Scale(gv_scale_factor, gv_center); 
    gv_d2d1_context->SetTransform(scale_matrix * translate_matrix); 
*********************************************************************************/    

    

    //  not required gv_d2d1_context->SetTransform(D2D1::Matrix3x2F::Identity());


    DX::ThrowIfFailed(
        gv_d2d1_context->EndDraw()
    );


    //    nice try, but needs work... m_d2d1_invalid = false; 


}  //  Closes VHG_D2D1::Render_d2d1(); 




