import { Activity, Terminal, Brain, Settings, HardDrive, Menu, X, Network } from 'lucide-react';
import { useState } from 'react';

const Layout = ({ children, activeView, setActiveView }) => {
    const [isMenuOpen, setIsMenuOpen] = useState(false);

    const NavItem = ({ name, icon: Icon, id }) => (
        <li 
            data-view={id}
            onClick={() => { setActiveView(id); setIsMenuOpen(false); }}
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
            {/* Mobile Header */}
            <div className="mobile-header" style={{ 
                display: 'none', 
                position: 'fixed', 
                top: 0, 
                left: 0, 
                width: '100%', 
                padding: '15px', 
                zIndex: 1001, 
                background: 'rgba(5, 10, 20, 0.9)',
                borderBottom: '1px solid var(--glass-border)',
                alignItems: 'center',
                justifyContent: 'space-between'
            }}>
                <h1 className="text-glow" style={{ margin: 0, fontSize: '16px', color: 'var(--accent-color)' }}>CORTEK OS</h1>
                <button 
                  onClick={() => setIsMenuOpen(!isMenuOpen)}
                  style={{ background: 'none', border: 'none', color: 'var(--accent-color)', cursor: 'pointer' }}
                >
                  {isMenuOpen ? <X size={24} /> : <Menu size={24} />}
                </button>
            </div>

            {/* Sidebar */}
            <div className={`sidebar ${isMenuOpen ? 'open' : ''}`} style={{ 
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
                        <NavItem name="Memory Map" icon={Network} id="memory" />
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
            <div className="main-content" style={{ flex: 1, position: 'relative', overflowY: 'auto', padding: '30px' }}>
                {children}
            </div>
        </div>
    );
};

export default Layout;
