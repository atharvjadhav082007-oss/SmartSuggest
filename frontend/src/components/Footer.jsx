import React from 'react';

export default function Footer() {
  return (
    <footer className="app-footer">
      <div className="oop-pills">
        <span className="pill" title="Dictionary, Trie, WhatsAppBot, SpellingGame encapsulate internals">
          🔒 Encapsulation
        </span>
        <span className="pill" title="Suggester defines pure virtual contract">
          🎭 Abstraction
        </span>
        <span className="pill" title="Autocomplete, SpellChecker, ParagraphChecker, GrammarChecker, SpellingGame derive Suggester">
          🧬 Inheritance
        </span>
        <span className="pill" title="Suggester* runtime dynamic dispatch across 5 engines">
          ⚡ Polymorphism
        </span>
        <span className="pill" title="ParagraphChecker HAS-A SpellChecker; WhatsAppBot HAS-A Engines">
          🧩 Composition
        </span>
        <span className="pill" title="TrieNode recursive destructor & LockGuard RAII mutex">
          🛡️ RAII
        </span>
      </div>
    </footer>
  );
}
