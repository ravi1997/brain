import React, { useState } from 'react';
import Layout from './components/Layout';
import Dashboard from './components/Dashboard';
import Cognition from './components/Cognition';

function App() {
  const [activeView, setActiveView] = useState('dashboard');

  const renderView = () => {
    switch(activeView) {
      case 'dashboard': return <Dashboard />;
      case 'cognition': return <Cognition />;
      default: return <Dashboard />;
    }
  };

  return (
    <Layout activeView={activeView} setActiveView={setActiveView}>
      {renderView()}
    </Layout>
  );
}

export default App;
