﻿#include <stdio.h>
#include <assert.h>
#include <glog/logging.h>
#include <utils/HttpUtility.h>
#include "../public/share/CommonDefine.h"
#include "Launch.h"

using namespace Firefly;

Launch::Launch()
{
    HttpUtility::Set_curl_global_init();
}

Launch::~Launch()
{
    Stop();
    HttpUtility::Set_curl_global_cleanup();
}

bool Launch::Start()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " startting service......";

    if (LaunchBase::Start() == false)
    {
        Stop();
        return false;
    }

    m_Timer.SetTimer( CONNECT_TIMER, 1, 1, SERVE_TYPE_CENTER );
    std::unique_lock <std::mutex> lck( m_mutMain );
    m_condMain.wait( lck );

    if( StartServer() == false )
    {
        LOG( ERROR ) << "net work service faild to stard......";
        Stop();
        return false;
    }

    return true;
}

bool Launch::Init()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " initializing service......";

    if( m_MsgServer.SetNetServer( &m_Bridge ) == false ) return false;
    if( m_MsgClient.SetClientEvent( &m_Bridge ) == false ) return false;
    if( m_Bridge.SetServer( &m_MsgServer ) == false ) return false;
    if( m_Bridge.SetBridgeHook( &m_BridgeHook ) == false ) return false;
    if( m_Timer.SetTimer( &m_Bridge ) == false ) return false;

    m_BridgeHook.m_pIMsgServer = dynamic_cast<IMsgServer *>( &m_MsgServer );
    m_BridgeHook.m_pIMsgClient = dynamic_cast<IMsgClient *>( &m_MsgClient );
    m_BridgeHook.m_pTimer = &m_Timer;
    m_BridgeHook.m_pLaunchBase = this;
    m_BridgeHook.m_pBridge = &m_Bridge;
    m_ContactCenter.SetMsgClient( dynamic_cast<IMsgClient *>( &m_MsgClient ) );
    m_BridgeHook.m_pContactCenter = &m_ContactCenter;
    m_BridgeHook.SetLoadSoNumber( m_nLoadSoNumber );
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " init good......";
    return true;
}

bool Launch::StartServer()
{
    //m_MsgServer.SetInfo( m_ServerInfo.nport(), m_ServerInfo.nmaxconnection() );

    if( m_MsgServer.Start() == false )
    {
        return false;
    }

    return true;
}

void Launch::SetLoadSoNumber( int     nLoadNumber )
{
    m_nLoadSoNumber = nLoadNumber;
}
