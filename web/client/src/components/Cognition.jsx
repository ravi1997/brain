import React from 'react';
import { useBrain } from '../hooks/useBrain';

const Cognition = () => {
    const { brainData, messages } = useBrain();

    // Filter thoughts only
    const thoughts = messages.filter(m => m.type === 'thought');

    return (
        <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: '20px', height: '100%' }}>
            {/* Thought Stream */}
            <div style={{ 
                border: '1px solid rgba(0, 240, 255, 0.3)', 
                background: 'rgba(0, 20, 40, 0.6)', 
                padding: '20px', overflowY: 'auto' 
            }}>
                <h3 style={{ color: '#00f0ff', marginTop: 0 }}>COGNITION STREAM</h3>
                {thoughts.map((t, i) => (
                    <div key={i} style={{ 
                        padding: '10px', marginBottom: '10px', 
                        borderLeft: '2px solid #00f0ff', 
                        background: 'rgba(0, 240, 255, 0.05)' 
                    }}>
                        {t.text}
                    </div>
                ))}
            </div>

            {/* Active Entities */}
            <div style={{ border: '1px solid rgba(0, 240, 255, 0.3)', background: 'rgba(0, 20, 40, 0.6)', padding: '20px' }}>
                <h3 style={{ color: '#00f0ff', marginTop: 0 }}>DETECTED ENTITIES</h3>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '10px' }}>
                    {/* Placeholder for entities */}
                    <span style={{ padding: '5px 10px', background: 'rgba(0, 240, 255, 0.2)', borderRadius: '15px' }}>Raven</span>
                    <span style={{ padding: '5px 10px', background: 'rgba(0, 240, 255, 0.2)', borderRadius: '15px' }}>Sylvia</span>
                </div>
            </div>
        </div>
    );
};

export default Cognition;
