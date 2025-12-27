import React, { useState } from 'react';

const Login = ({ onLogin }) => {
  const [password, setPassword] = useState('');
  const [error, setError] = useState(false);

  const handleSubmit = (e) => {
    e.preventDefault();
    if (password === 'brain') {
      onLogin();
    } else {
      setError(true);
      setTimeout(() => setError(false), 3000);
    }
  };

  return (
    <div className="login-overlay glass-panel">
      <div className="login-box">
        <h1 className="text-glow">BRAIN // AUTHENTICATION</h1>
        <p>SYSTEM SECURITY ACTIVE. PLEASE IDENTIFY.</p>
        <form onSubmit={handleSubmit}>
          <input 
            type="password" 
            placeholder="ACCESS KEY" 
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            className="glass-input"
            id="password-input"
            style={{ borderColor: error ? 'var(--danger-color)' : 'var(--border-color)', transition: 'border-color 0.3s' }}
          />
          {error && <div style={{ color: 'var(--danger-color)', fontSize: '10px', marginBottom: '10px', fontWeight: 'bold' }}>SECURE CHANNEL ACCESS DENIED</div>}
          <button type="submit" className="glass-button neon-border">LOGIN</button>
        </form>
      </div>
    </div>
  );
};

export default Login;
