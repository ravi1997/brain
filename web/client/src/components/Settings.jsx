import React from 'react';

const Settings = ({ theme, toggleTheme }) => {
    return (
        <div style={{ padding: '20px' }}>
            <h2 style={{ color: 'var(--accent-color)', marginBottom: '30px' }}>SYSTEM SETTINGS</h2>
            
            <div className="glass-panel" style={{ padding: '30px', maxWidth: '600px' }}>
                <div style={{ marginBottom: '20px', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                    <span>INTERFACE THEME</span>
                    <button 
                        onClick={toggleTheme}
                        style={{
                            background: 'var(--accent-color)',
                            color: '#000',
                            border: 'none',
                            padding: '5px 15px',
                            borderRadius: '4px',
                            cursor: 'pointer',
                            fontWeight: 'bold',
                            textTransform: 'uppercase',
                            fontSize: '12px'
                        }}
                    >
                        {theme === 'dark' ? 'LIGHT MODE' : 'DARK MODE'}
                    </button>
                </div>

                <div style={{ marginBottom: '20px' }}>
                    <label style={{ display: 'block', marginBottom: '10px', opacity: 0.7 }}>CRITICALITY LEVEL</label>
                    <input type="range" style={{ width: '100%', accentColor: 'var(--accent-color)' }} />
                </div>
                
                <div style={{ marginBottom: '20px', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                    <span>AUTONOMOUS MODE</span>
                    <div style={{ 
                        width: '50px', height: '24px', background: 'var(--accent-color)', 
                        borderRadius: '12px', position: 'relative', cursor: 'pointer' 
                    }}>
                        <div style={{ 
                            width: '20px', height: '20px', background: '#fff', 
                            borderRadius: '50%', position: 'absolute', right: '2px', top: '2px' 
                        }} />
                    </div>
                </div>

                <button style={{
                    background: 'transparent',
                    border: '1px solid var(--accent-color)',
                    color: 'var(--accent-color)',
                    padding: '10px 20px',
                    borderRadius: '4px',
                    cursor: 'pointer',
                    marginTop: '20px'
                }}>
                    FACTORY RESET
                </button>
            </div>
        </div>
    );
};

export default Settings;
