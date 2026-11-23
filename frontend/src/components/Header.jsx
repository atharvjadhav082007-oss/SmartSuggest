import React from 'react';

export default function Header() {
  return (
    <header className="app-header">
      <div className="header-top">
        <div className="logo-box">
          <div className="logo-icon logo-spin">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5">
              <polygon points="12 2 2 7 12 12 22 7 12 2"></polygon>
              <polyline points="2 17 12 22 22 17"></polyline>
              <polyline points="2 12 12 17 22 12"></polyline>
            </svg>
          </div>
          <span className="logo-text">
            SmartSuggest<span className="logo-accent">.cpp</span>
            <span style={{ fontSize: '0.65rem', marginLeft: '6px', padding: '2px 6px', background: '#e0e7ff', color: '#4338ca', borderRadius: '4px', fontWeight: 700 }}>React 18</span>
          </span>
        </div>
        <div className="status-pill">
          <span className="pulse-dot"></span>
          <span className="status-label">C++17 Engine Live</span>
        </div>
      </div>
      <h1 className="main-title fade-in-up">Autocomplete, Spell Checker, Grammar & Game</h1>
      <p className="subtitle fade-in-up delay-1">
        High-performance engine built with pure Object-Oriented C++ architecture & interactive React 18 UI
      </p>
    </header>
  );
}
