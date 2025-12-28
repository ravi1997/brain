import React, { useState } from 'react';
import { useBrain } from '../hooks/useBrain';
import { Zap, Activity, Target, Cpu } from 'lucide-react';
import Tooltip from './Tooltip';

const Dashboard = () => {
    const { status, brainData } = useBrain();
    
    const StatCard = ({ title, value, icon: Icon, id, color = 'var(--accent-color)', tooltip }) => (
        <Tooltip text={tooltip} position="bottom">
            <div id={id} className="glass-panel" style={{ padding: '25px', position: 'relative', overflow: 'hidden', cursor: 'help' }}>
                <div className="scan-line" />
                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '15px' }}>
                    <span style={{ fontSize: '10px', color: 'rgba(255,255,255,0.4)', fontWeight: 'bold' }}>{title}</span>
                    <Icon size={16} style={{ color }} />
                </div>
                <div style={{ fontSize: '32px', fontFamily: 'var(--font-mono)', color }}>{value}</div>
                <div style={{ height: '4px', background: 'rgba(255,255,255,0.1)', marginTop: '15px', borderRadius: '2px' }}>
                    <div style={{ 
                        height: '100%', 
                        width: typeof value === 'string' && value.includes('%') ? value : '100%', 
                        background: color, 
                        boxShadow: `0 0 10px ${color}` 
                    }} />
                </div>
            </div>
        </Tooltip>
    );

    return (
        <div style={{ display: 'flex', flexDirection: 'column', gap: '30px' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <div>
                    <h2 id="page-title" className="text-glow" style={{ margin: 0, fontSize: '28px', color: '#fff' }}>SYSTEM OVERVIEW</h2>
                    <p style={{ margin: '5px 0 0 0', opacity: 0.5, fontSize: '12px', letterSpacing: '1px' }}>REAL-TIME COGNITIVE TELEMETRY</p>
                </div>
                <div className="glass-panel" style={{ padding: '10px 20px', display: 'flex', alignItems: 'center', gap: '10px' }}>
                    <div className="pulse" style={{ width: '8px', height: '8px', borderRadius: '50%', background: status === 'connected' ? 'var(--success-color)' : 'var(--danger-color)' }} />
                    <span style={{ fontSize: '12px', fontFamily: 'var(--font-mono)' }}>LINK STATUS: {status.toUpperCase()}</span>
                </div>
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(240px, 1fr))', gap: '20px' }}>
                <StatCard 
                    id="dash-energy" 
                    title="ENERGY RESERVES" 
                    value={`${(brainData.emotions?.energy * 100 || 0).toFixed(0)}%`} 
                    icon={Zap} 
                    color="var(--accent-color)" 
                    tooltip="CURRENT SYSTEM POWER LEVEL"
                />
                <StatCard 
                    id="dash-happiness" 
                    title="COGNITIVE SYNC" 
                    value={`${(brainData.emotions?.happiness * 100 || 0).toFixed(0)}%`} 
                    icon={Activity} 
                    color="var(--success-color)" 
                    tooltip="ALIGNMENT WITH USER OBJECTIVES"
                />
                <StatCard 
                    id="dash-boredom" 
                    title="NEURAL LOAD" 
                    value={`${(brainData.emotions?.boredom * 100 || 0).toFixed(0)}%`} 
                    icon={Cpu} 
                    color="var(--warning-color)" 
                    tooltip="ACTIVE PROCESSING SATURATION"
                />
            </div>

            <div className="glass-panel" style={{ padding: '30px', flex: 1 }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '15px', marginBottom: '20px' }}>
                    <Target size={20} style={{ color: 'var(--accent-color)' }} />
                    <h3 style={{ margin: 0 }}>CURRENT FOCUS</h3>
                </div>
                <div style={{ 
                    background: 'rgba(0,0,0,0.2)', 
                    padding: '25px', 
                    borderRadius: '4px', 
                    fontFamily: 'var(--font-mono)',
                    fontSize: '18px',
                    lineHeight: '1.6',
                    borderLeft: '4px solid var(--accent-color)'
                }}>
                    {brainData.thought || "AWAITING NEURAL SIGNALS..."}
                </div>
            </div>
        </div>
    );
};

export default Dashboard;
