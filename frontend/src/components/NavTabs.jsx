import React from 'react';

export default function NavTabs({ activeTab, onSelectTab }) {
  const tabs = [
    {
      id: 'autocomplete',
      title: 'Autocomplete',
      sub: 'Trie Prefix Search',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <circle cx="11" cy="11" r="8"></circle>
          <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
        </svg>
      )
    },
    {
      id: 'spellcheck',
      title: 'Spell Check',
      sub: 'Levenshtein Distance',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path>
          <polyline points="9 12 11 14 15 10"></polyline>
        </svg>
      )
    },
    {
      id: 'paragraph',
      title: 'Paragraph',
      sub: 'Whole-Text Check',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <line x1="17" y1="10" x2="3" y2="10"></line>
          <line x1="21" y1="6" x2="3" y2="6"></line>
          <line x1="21" y1="14" x2="3" y2="14"></line>
          <line x1="17" y1="18" x2="3" y2="18"></line>
        </svg>
      )
    },
    {
      id: 'grammar',
      title: 'Grammar',
      sub: 'Rule-Based Check',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M4 7V4h16v3"></path>
          <path d="M9 20h6"></path>
          <path d="M12 4v16"></path>
        </svg>
      )
    },
    {
      id: 'game',
      title: 'Spelling Game',
      sub: 'Challenge Mode',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"></polygon>
        </svg>
      )
    }
  ];

  const handleTabClick = (e, tabId) => {
    const btn = e.currentTarget;
    const ripple = document.createElement('span');
    ripple.className = 'ripple';
    const rect = btn.getBoundingClientRect();
    const size = Math.max(rect.width, rect.height);
    ripple.style.width = ripple.style.height = `${size}px`;
    ripple.style.left = `${e.clientX - rect.left - size / 2}px`;
    ripple.style.top = `${e.clientY - rect.top - size / 2}px`;
    btn.appendChild(ripple);
    ripple.addEventListener('animationend', () => ripple.remove());

    onSelectTab(tabId);
  };

  return (
    <nav className="tabs-wrapper" role="tablist">
      {tabs.map((tab) => (
        <button
          key={tab.id}
          className={`tab-btn ${activeTab === tab.id ? 'active' : ''}`}
          role="tab"
          aria-selected={activeTab === tab.id}
          onClick={(e) => handleTabClick(e, tab.id)}
        >
          <div className="tab-icon-wrap">{tab.icon}</div>
          <span className="tab-title">{tab.title}</span>
          <span className="tab-sub">{tab.sub}</span>
        </button>
      ))}
    </nav>
  );
}
