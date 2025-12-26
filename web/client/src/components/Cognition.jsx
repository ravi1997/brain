import React from 'react';
import { useBrain } from '../hooks/useBrain';
import NeuronVisualizer from './NeuronVisualizer';

const Cognition = () => {
    const { status, messages } = useBrain();

    // Filter thoughts only
    const thoughts = messages.filter(m => m.type === 'thought');

    return (
        <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: '30px', height: '100%' }}>
            {/* Neural Activity Panel */}
            <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
                <div className="glass-panel" style={{ padding: '20px' }}>
                    <h3 style={{ color: 'var(--accent-color)', marginTop: 0 }}>NEURAL ACTIVITY</h3>
                    <div id="neural-canvas" style={{ width: '100%', borderRadius: '4px', overflow: 'hidden', border: '1px solid var(--glass-border)' }}>
                        <NeuronVisualizer brainState={status} />
                    </div>
                </div>

                <div className="glass-panel" style={{ flex: 1, padding: '20px', overflowY: 'auto' }}>
                    <h3 style={{ color: 'var(--accent-color)', marginTop: 0 }}>COGNITION STREAM</h3>
                    <div style={{ display: 'flex', flexDirection: 'column', gap: '10px', marginTop: '15px' }}>
                        {thoughts.map((t, i) => (
                            <div key={i} style={{ 
                                padding: '15px', 
                                borderLeft: '2px solid var(--accent-color)', 
                                background: 'rgba(0, 240, 255, 0.05)',
                                fontFamily: 'var(--font-mono)',
                                fontSize: '14px'
                            }}>
                                <span style={{ opacity: 0.3, marginRight: '10px' }}>&gt;</span>
                                {t.text}
                            </div>
                        ))}
                    </div>
                </div>
            </div>

            {/* Active Entities */}
            <div className="glass-panel" style={{ padding: '20px' }}>
                <h3 style={{ color: 'var(--accent-color)', marginTop: 0 }}>DETECTED ENTITIES</h3>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '10px', marginTop: '15px' }}>
                    {['RAVEN', 'SYLVIA', 'CORTEX', 'PROTOCOL-7'].map(entity => (
                        <span key={entity} style={{ 
                            padding: '6px 14px', 
                            background: 'rgba(0, 240, 255, 0.1)', 
                            border: '1px solid var(--glass-border)',
                            borderRadius: '15px',
                            fontSize: '11px',
                            fontWeight: 'bold',
                            letterSpacing: '1px'
                        }}>{entity}</span>
                    ))}
                </div>
            </div>
        </div>
    );
};

export default Cognition;
