import React, { useState } from 'react';
import Layout from './components/Layout';
import Dashboard from './components/Dashboard';
import Cognition from './components/Cognition';
import Terminal from './components/Terminal';
import System from './components/System';
import Settings from './components/Settings';

import Login from './components/Login';

function App() {
  const [activeView, setActiveView] = useState('dashboard');
  const [theme, setTheme] = useState('dark');
  const [isAuthenticated, setIsAuthenticated] = useState(localStorage.getItem('brain_auth') === 'true');

  const toggleTheme = () => {
    const newTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(newTheme);
    document.documentElement.setAttribute('data-theme', newTheme);
  };

  const handleLogin = () => {
    setIsAuthenticated(true);
    localStorage.setItem('brain_auth', 'true');
  };

  const renderView = () => {
    switch(activeView) {
      case 'dashboard': return <Dashboard />;
      case 'neural': return <Cognition />;
      case 'terminal': return <Terminal />;
      case 'system': return <System />;
      case 'settings': return <Settings theme={theme} toggleTheme={toggleTheme} />;
      default: return <Dashboard />;
    }
  };

  return (
    <>
      {!isAuthenticated && <Login onLogin={handleLogin} />}
      <Layout activeView={activeView} setActiveView={setActiveView}>
        {renderView()}
      </Layout>
    </>
  );
}

export default App;
