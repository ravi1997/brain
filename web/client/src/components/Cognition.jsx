import React from 'react';
import { useBrain } from '../hooks/useBrain';
import NeuronVisualizer from './NeuronVisualizer';
import Tooltip from './Tooltip';

const Cognition = () => {
    const { status, messages, brainData } = useBrain();

    // Filter thoughts only
    const thoughts = messages.filter(m => m.type === 'thought');
    const entities = brainData.entities || ['RAVEN', 'SYLVIA', 'CORTEX', 'PROTOCOL-7'];

    return (
        <div className="grid-2" style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: '30px', height: '100%' }}>
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
                            <Tooltip key={i} text={`CONFIDENCE: ${(Math.random() * 0.2 + 0.8).toFixed(2)}`}>
                                <div style={{ 
                                    padding: '15px', 
                                    borderLeft: '2px solid var(--accent-color)', 
                                    background: 'rgba(0, 240, 255, 0.05)',
                                    fontFamily: 'var(--font-mono)',
                                    fontSize: '14px',
                                    cursor: 'help'
                                }}>
                                    <span style={{ opacity: 0.3, marginRight: '10px' }}>&gt;</span>
                                    {t.text}
                                </div>
                            </Tooltip>
                        ))}
                    </div>
                </div>
            </div>

            {/* Right Panel: Entities & Sensory */}
            <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
                <div className="glass-panel" style={{ padding: '20px' }}>
                    <h3 style={{ color: 'var(--accent-color)', marginTop: 0 }}>DETECTED ENTITIES</h3>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '10px', marginTop: '15px' }}>
                        {entities.map(entity => (
                            <Tooltip key={entity} text={`SEMANTIC CATEGORY: ${Math.random() > 0.5 ? 'SYSTEM' : 'ENTITY'}`}>
                                <span style={{ 
                                    padding: '6px 14px', 
                                    background: 'rgba(0, 240, 255, 0.1)', 
                                    border: '1px solid var(--glass-border)',
                                    borderRadius: '15px',
                                    fontSize: '11px',
                                    fontWeight: 'bold',
                                    letterSpacing: '1px',
                                    cursor: 'help'
                                }}>{entity}</span>
                            </Tooltip>
                        ))}
                    </div>
                </div>

                <div className="glass-panel" style={{ padding: '20px' }}>
                    <h3 style={{ color: 'var(--accent-color)', marginTop: 0 }}>SENSORY MONITORING</h3>
                    <div style={{ display: 'flex', flexDirection: 'column', gap: '15px', marginTop: '15px' }}>
                        {(brainData.sensory_activity || []).map((sens, idx) => (
                            <div key={idx} style={{ padding: '10px', background: 'rgba(255, 255, 255, 0.05)', borderRadius: '4px' }}>
                                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '5px' }}>
                                    <span style={{ fontSize: '10px', color: '#ec4899', fontWeight: 'bold' }}>{sens.name.toUpperCase()}</span>
                                    <span style={{ fontSize: '10px', color: 'var(--accent-color)' }}>FOCUS: {(sens.focus * 100).toFixed(0)}%</span>
                                </div>
                                <div style={{ height: '4px', width: '100%', background: 'rgba(255,255,255,0.1)', borderRadius: '2px', overflow: 'hidden' }}>
                                    <div style={{ 
                                        height: '100%', 
                                        width: `${sens.focus * 100}%`, 
                                        background: 'var(--accent-color)',
                                        boxShadow: '0 0 10px var(--accent-color)',
                                        transition: 'width 0.3s ease'
                                    }} />
                                </div>
                            </div>
                        ))}
                        {(!brainData.sensory_activity || brainData.sensory_activity.length === 0) && (
                            <div style={{ fontSize: '12px', opacity: 0.5, fontStyle: 'italic' }}>No active sensory units detected.</div>
                        )}
                    </div>
                </div>
            </div>
        </div>
    );
};

export default Cognition;
