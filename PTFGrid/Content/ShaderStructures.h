#pragma once

 

namespace VHG
{

    //   
    //    Constant buffer used to send 
    //    Model-View-Projection matrix data 
    //    to the vertex shader 
    // 

    struct VHG_ConBuf_MVP_Struct 
    {
        DirectX::XMFLOAT4X4     model;
        DirectX::XMFLOAT4X4     view;
        DirectX::XMFLOAT4X4     projection; 
        DirectX::XMUINT4        animator_count; 
    };




    struct VHG_VertexPosTex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 texco;
    }; 




    struct VHG_Instance
    {
        DirectX::XMFLOAT3    inst_pos;    
    };


}  //  Closes namespace VHG; 


