import React, { useState } from 'react';
import ParticleBackground from './components/ParticleBackground';
import Header from './components/Header';
import NavTabs from './components/NavTabs';
import AutocompleteView from './components/AutocompleteView';
import SpellCheckView from './components/SpellCheckView';
import ParagraphView from './components/ParagraphView';
import GrammarView from './components/GrammarView';
import GameView from './components/GameView';
import OopInspector from './components/OopInspector';
import Footer from './components/Footer';

export default function App() {
  const [activeTab, setActiveTab] = useState('autocomplete');
  const [latency, setLatency] = useState('ready');

  const handleLatency = (val) => {
    setLatency(val);
  };

  const handleTabChange = (newTab) => {
    setActiveTab(newTab);
    setLatency('ready');
  };

  return (
    <>
      <ParticleBackground />

      <div className="app-container">
        <Header />

        <NavTabs activeTab={activeTab} onSelectTab={handleTabChange} />

        <main className="glass-card">
          {activeTab === 'autocomplete' && (
            <AutocompleteView onMeasureLatency={handleLatency} />
          )}

          {activeTab === 'spellcheck' && (
            <SpellCheckView onMeasureLatency={handleLatency} />
          )}

          {activeTab === 'paragraph' && (
            <ParagraphView onMeasureLatency={handleLatency} />
          )}

          {activeTab === 'grammar' && (
            <GrammarView onMeasureLatency={handleLatency} />
          )}

          {activeTab === 'game' && (
            <GameView />
          )}

          <OopInspector activeTab={activeTab} latency={latency} />
        </main>

        <Footer />
      </div>
    </>
  );
}
