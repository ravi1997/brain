import React from 'react';
import { useBrain } from '../hooks/useBrain';

const Terminal = () => {
    const { messages, sendMessage } = useBrain();
    const [input, setInput] = React.useState('');

    const handleKeyDown = (e) => {
        if (e.key === 'Enter' && input.trim()) {
            sendMessage(input);
            setInput('');
        }
    };

    return (
        <div style={{ display: 'flex', flexDirection: 'column', height: '100%', gap: '10px' }}>
            <div 
                id="term-out"
                style={{
                    flex: 1,
                    background: 'rgba(0, 0, 0, 0.4)',
                    border: '1px solid var(--border-color)',
                    borderRadius: '4px',
                    padding: '15px',
                    overflowY: 'auto',
                    fontFamily: 'var(--font-mono)',
                    fontSize: '14px',
                    color: '#fff'
                }}
            >
                {messages.map((m, i) => (
                    <div key={i} style={{ marginBottom: '4px', color: m.type === 'user' ? '#00f0ff' : '#a0c0e0' }}>
                        <span style={{ opacity: 0.5 }}>[{new Date().toLocaleTimeString()}]</span>
                        {m.type === 'user' ? ' root@brain:~# ' : ' # '}
                        {m.text}
                    </div>
                ))}
            </div>
            <div style={{ display: 'flex', gap: '10px', alignItems: 'center' }}>
                <span style={{ fontFamily: 'var(--font-mono)', color: '#00f0ff' }}>root@brain:~#</span>
                <input
                    id="term-in"
                    type="text"
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    onKeyDown={handleKeyDown}
                    style={{
                        flex: 1,
                        background: 'transparent',
                        border: 'none',
                        borderBottom: '1px solid var(--accent-color)',
                        color: '#fff',
                        fontFamily: 'var(--font-mono)',
                        fontSize: '14px',
                        outline: 'none',
                        padding: '5px'
                    }}
                    autoFocus
                />
            </div>
        </div>
    );
};

export default Terminal;
