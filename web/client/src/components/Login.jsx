import React, { useState } from 'react';

const Login = ({ onLogin }) => {
  const [password, setPassword] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    if (password === 'brain') {
      onLogin();
    } else {
      alert('ACCESS DENIED');
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
          />
          <button type="submit" className="glass-button neon-border">LOGIN</button>
        </form>
      </div>
    </div>
  );
};

export default Login;
