import React from 'react';
import { Activity, Terminal, Brain, Settings } from 'lucide-react';

const Layout = ({ children, activeView, setActiveView }) => {
    const NavItem = ({ name, icon: Icon, id }) => (
        <div 
            onClick={() => setActiveView(id)}
            style={{
                display: 'flex', alignItems: 'center', padding: '15px',
                cursor: 'pointer',
                color: activeView === id ? '#00f0ff' : '#a0c0e0',
                borderLeft: activeView === id ? '4px solid #00f0ff' : '4px solid transparent',
                background: activeView === id ? 'rgba(0, 240, 255, 0.1)' : 'transparent'
            }}
        >
            <Icon size={20} style={{ marginRight: '10px' }} />
            <span style={{ fontSize: '14px', textTransform: 'uppercase', letterSpacing: '1px' }}>{name}</span>
        </div>
    );

    return (
        <div style={{ display: 'flex', height: '100vh', background: 'radial-gradient(circle at center, #0a1428 0%, #050a14 100%)' }}>
            {/* Sidebar */}
            <div style={{ width: '250px', borderRight: '1px solid rgba(0, 240, 255, 0.2)', background: 'rgba(5, 10, 20, 0.9)' }}>
                <div style={{ padding: '20px', borderBottom: '1px solid rgba(0, 240, 255, 0.2)' }}>
                    <h1 style={{ margin: 0, fontSize: '18px', color: '#00f0ff', textShadow: '0 0 10px #00f0ff' }}>BRAINTOWER OS</h1>
                    <div style={{ fontSize: '10px', color: '#666', marginTop: '5px' }}>V2.0.0 REACT CORE</div>
                </div>
                <div style={{ marginTop: '20px' }}>
                    <NavItem name="Dashboard" icon={Activity} id="dashboard" />
                    <NavItem name="Cognition" icon={Brain} id="cognition" />
                    <NavItem name="Terminal" icon={Terminal} id="terminal" />
                    <NavItem name="Settings" icon={Settings} id="settings" />
                </div>
            </div>

            {/* Main Content */}
            <div style={{ flex: 1, overflow: 'hidden', padding: '20px', position: 'relative' }}>
                {/* Scanlines Effect */}
                <div style={{
                    position: 'absolute', top: 0, left: 0, right: 0, bottom: 0,
                    background: 'linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.1) 50%)',
                    backgroundSize: '100% 4px', pointerEvents: 'none', zIndex: 9999
                }}></div>
                {children}
            </div>
        </div>
    );
};

export default Layout;
