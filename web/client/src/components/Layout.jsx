import React from 'react';
import { Activity, Terminal, Brain, Settings, HardDrive } from 'lucide-react';

const Layout = ({ children, activeView, setActiveView }) => {
    const NavItem = ({ name, icon: Icon, id }) => (
        <li 
            data-view={id}
            onClick={() => setActiveView(id)}
            className={activeView === id ? 'active' : ''}
            style={{
                display: 'flex', alignItems: 'center', padding: '15px 25px',
                cursor: 'pointer',
                listStyle: 'none',
                color: activeView === id ? 'var(--accent-color)' : 'var(--text-color)',
                borderLeft: activeView === id ? '4px solid var(--accent-color)' : '4px solid transparent',
                background: activeView === id ? 'rgba(0, 240, 255, 0.1)' : 'transparent',
                transition: 'all 0.3s ease',
                opacity: activeView === id ? 1 : 0.7
            }}
        >
            <Icon size={20} style={{ marginRight: '15px', color: activeView === id ? 'var(--accent-color)' : 'inherit' }} />
            <span style={{ fontSize: '13px', fontWeight: '500', letterSpacing: '2px' }}>{name}</span>
        </li>
    );

    return (
        <div className="scanlines" style={{ display: 'flex', height: '100vh', width: '100vw', background: 'var(--bg-color)', position: 'relative', overflow: 'hidden' }}>
            {/* Sidebar */}
            <div style={{ 
                width: '280px', 
                borderRight: '1px solid var(--glass-border)', 
                background: 'rgba(5, 10, 20, 0.95)',
                zIndex: 10,
                display: 'flex',
                flexDirection: 'column'
            }}>
                <div style={{ padding: '30px 25px', borderBottom: '1px solid var(--glass-border)' }}>
                    <h1 className="text-glow" style={{ margin: 0, fontSize: '20px', color: 'var(--accent-color)' }}>CORTEK OS</h1>
                    <div style={{ fontSize: '10px', color: 'rgba(0, 240, 255, 0.5)', marginTop: '5px', letterSpacing: '1px' }}>VERSION 2.1.0-REACT</div>
                </div>
                
                <nav style={{ flex: 1, paddingTop: '20px' }}>
                    <ul style={{ margin: 0, padding: 0 }}>
                        <NavItem name="Overview" icon={Activity} id="dashboard" />
                        <NavItem name="Neural Link" icon={Brain} id="neural" />
                        <NavItem name="Storage" icon={HardDrive} id="storage" />
                        <NavItem name="System" icon={HardDrive} id="system" />
                        <NavItem name="Terminal" icon={Terminal} id="terminal" />
                        <NavItem name="Settings" icon={Settings} id="settings" />
                    </ul>
                </nav>

                <div style={{ padding: '20px', fontSize: '10px', opacity: 0.3, borderTop: '1px solid var(--glass-border)' }}>
                    SECURE CONNECTION VERIFIED
                </div>
            </div>

            {/* Main Content */}
            <div style={{ flex: 1, position: 'relative', overflowY: 'auto', padding: '30px' }}>
                {children}
            </div>
        </div>
    );
};

export default Layout;
