// Item 51, 25: TypeScript Tokenizer Logic
export class Tokenizer {
  private stopWords: Set<string>;

  constructor() {
    this.stopWords = new Set(["the", "a", "an", "is", "of"]); // Stub
  }

  public tokenize(text: string): string[] {
    return text
      .toLowerCase()
      .split(/\s+/)
      .filter((w) => !this.stopWords.has(w) && w.length > 0);
  }

  public async tokenizeAsync(text: string): Promise<string[]> {
    // Simulating async operation
    return new Promise((resolve) => {
      setTimeout(() => resolve(this.tokenize(text)), 10);
    });
  }
}
