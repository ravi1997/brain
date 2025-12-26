import React from 'react';
import { 
    LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, AreaChart, Area 
} from 'recharts';
import { useBrain } from '../hooks/useBrain';

const System = () => {
    const { brainData } = useBrain();
    
    // Mock historical data for charts
    const cpuData = Array.from({ length: 20 }, (_, i) => ({
        time: i,
        value: 20 + Math.random() * 40
    }));

    const memData = Array.from({ length: 20 }, (_, i) => ({
        time: i,
        value: 40 + Math.random() * 30
    }));

    return (
        <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '20px', height: '100%', padding: '10px' }}>
            <div className="glass-panel" style={{ padding: '20px' }}>
                <h3 style={{ marginBottom: '20px', color: 'var(--accent-color)' }}>CPU USAGE</h3>
                <div id="chart-cpu" style={{ width: '100%', height: '250px' }}>
                    <ResponsiveContainer width="100%" height="100%">
                        <AreaChart data={cpuData}>
                            <defs>
                                <linearGradient id="colorCpu" x1="0" y1="0" x2="0" y2="1">
                                    <stop offset="5%" stopColor="var(--accent-color)" stopOpacity={0.8}/>
                                    <stop offset="95%" stopColor="var(--accent-color)" stopOpacity={0}/>
                                </linearGradient>
                            </defs>
                            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
                            <XAxis dataKey="time" hide />
                            <YAxis stroke="rgba(255,255,255,0.5)" />
                            <Tooltip contentStyle={{ background: 'rgba(0,0,0,0.8)', border: '1px solid var(--accent-color)' }} />
                            <Area type="monotone" dataKey="value" stroke="var(--accent-color)" fillOpacity={1} fill="url(#colorCpu)" />
                        </AreaChart>
                    </ResponsiveContainer>
                </div>
                <div id="sys-cpu" style={{ marginTop: '10px', fontSize: '24px', fontFamily: 'var(--font-mono)' }}>
                    {Math.round(cpuData[19].value)}%
                </div>
            </div>

            <div className="glass-panel" style={{ padding: '20px' }}>
                <h3 style={{ marginBottom: '20px', color: 'var(--accent-color)' }}>MEMORY UTILIZATION</h3>
                <div id="chart-mem" style={{ width: '100%', height: '250px' }}>
                    <ResponsiveContainer width="100%" height="100%">
                        <LineChart data={memData}>
                            <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
                            <XAxis dataKey="time" hide />
                            <YAxis stroke="rgba(255,255,255,0.5)" />
                            <Tooltip contentStyle={{ background: 'rgba(0,0,0,0.8)', border: '1px solid var(--accent-color)' }} />
                            <Line type="monotone" dataKey="value" stroke="var(--success-color)" strokeWidth={2} dot={false} />
                        </LineChart>
                    </ResponsiveContainer>
                </div>
                <div id="sys-mem" style={{ marginTop: '10px', fontSize: '24px', fontFamily: 'var(--font-mono)' }}>
                    {Math.round(memData[19].value)}%
                </div>
            </div>
        </div>
    );
};

export default System;
