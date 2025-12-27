import React, { useState } from 'react';

const Tooltip = ({ children, text, position = 'top' }) => {
    const [visible, setVisible] = useState(false);
    
    const getPositionStyles = () => {
        switch(position) {
            case 'bottom':
                return { top: '100%', left: '50%', transform: 'translateX(-50%)', marginTop: '10px' };
            case 'left':
                return { top: '50%', right: '100%', transform: 'translateY(-50%)', marginRight: '10px' };
            case 'right':
                return { top: '50%', left: '100%', transform: 'translateY(-50%)', marginLeft: '10px' };
            default: // top
                return { bottom: '100%', left: '50%', transform: 'translateX(-50%)', marginBottom: '10px' };
        }
    };

    return (
        <div style={{ position: 'relative', display: 'inline-block' }} onMouseEnter={() => setVisible(true)} onMouseLeave={() => setVisible(false)}>
            {children}
            {visible && (
                <div style={{
                    position: 'absolute',
                    background: 'rgba(5, 10, 20, 0.95)',
                    border: '1px solid var(--accent-color)',
                    padding: '8px 12px',
                    borderRadius: '4px',
                    fontSize: '10px',
                    color: 'var(--accent-color)',
                    whiteSpace: 'nowrap',
                    zIndex: 1000,
                    boxShadow: '0 0 10px rgba(0, 240, 255, 0.3)',
                    ...getPositionStyles()
                }}>
                    {text}
                </div>
            )}
        </div>
    );
};

export default Tooltip;
