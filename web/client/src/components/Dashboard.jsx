import React, { useState } from 'react';
import { useBrain } from '../hooks/useBrain';
import NeuronVisualizer from './NeuronVisualizer';

const Dashboard = () => {
    const { status, messages, sendMessage } = useBrain();
    const [input, setInput] = useState("");
    
    console.log("%c[Dashboard] Current status: " + status, "color: yellow; font-weight: bold; font-size: 16px;");

    const handleSend = (e) => {
        if (e.key === 'Enter' && input.trim()) {
            sendMessage(input);
            setInput("");
        }
    };

    return (
        <div style={{ display: 'grid', gridTemplateColumns: '1fr 300px', gap: '20px', height: '100%' }}>
            {/* Main Panel */}
            <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
                {/* Status Bar */}
                <div style={{ 
                    padding: '15px', border: '1px solid rgba(0, 240, 255, 0.3)', 
                    background: 'rgba(0, 20, 40, 0.6)', backdropFilter: 'blur(5px)',
                    display: 'flex', justifyContent: 'space-between'
                }}>
                    <span style={{ color: status === 'connected' ? '#0f0' : '#f00' }}>
                        ● SYSTEM STATUS: {status.toUpperCase()}
                    </span>
                    <span>UPTIME: 00:00:00</span>
                </div>

                {/* Logs / Chat */}
                <div style={{ 
                    flex: 1, border: '1px solid rgba(0, 240, 255, 0.3)', 
                    background: 'rgba(0, 10, 20, 0.8)', padding: '20px',
                    overflowY: 'auto', fontFamily: 'monospace'
                }}>
                    {messages.map((m, i) => (
                        <div key={i} style={{ 
                            marginBottom: '8px', 
                            color: m.type === 'user' ? '#fff' : m.type === 'thought' ? '#a0c0e0' : '#00f0ff' 
                        }}>
                            <span style={{ opacity: 0.5 }}>[{new Date().toLocaleTimeString()}]</span> 
                            {m.type === 'user' ? ' > ' : ' # '}
                            {m.text}
                        </div>
                    ))}
                </div>

                {/* Neuron Activity Visualization */}
                <div style={{ 
                    border: '1px solid rgba(0, 240, 255, 0.3)', 
                    background: 'rgba(0, 10, 20, 0.8)',
                    overflow: 'hidden'
                }}>
                    <div style={{ padding: '10px', borderBottom: '1px solid rgba(0, 240, 255, 0.3)' }}>
                        <h3 style={{ margin: 0, color: '#00f0ff', fontSize: '14px' }}>NEURAL ACTIVITY</h3>
                    </div>
                    <NeuronVisualizer brainState={status} />
                </div>

                {/* Input */}
                <input 
                    type="text" 
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    onKeyDown={handleSend}
                    placeholder="Enter command..."
                    style={{
                        background: 'rgba(0, 20, 40, 0.8)', border: '1px solid #00f0ff',
                        color: '#fff', padding: '15px', fontSize: '16px', fontFamily: 'monospace',
                        outline: 'none'
                    }}
                />
            </div>

            {/* Right Panel (Stats) */}
            <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
                <div style={{ padding: '20px', border: '1px solid rgba(0, 240, 255, 0.3)', background: 'rgba(0, 20, 40, 0.6)' }}>
                    <h3 style={{ margin: '0 0 15px 0', borderBottom: '1px solid #00f0ff', paddingBottom: '5px' }}>VITALS</h3>
                    <div>ENERGY: 100%</div>
                    <div>HAPPINESS: 50%</div>
                    <div>MEMORY: 1024 KB</div>
                </div>
            </div>
        </div>
    );
};

export default Dashboard;
