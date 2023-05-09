module.exports = {
  env: {
    node: true,
    es2021: true,
  },
  extends: ['eslint:recommended', 'prettier'],
  overrides: [],
  parserOptions: {
    ecmaVersion: 'latest',
    sourceType: 'module',
  },

  rules: {
    // Moved to prettier for stylistic rules,
    // see https://typescript-eslint.io/linting/troubleshooting/formatting/
  },
  plugins: [],
};
