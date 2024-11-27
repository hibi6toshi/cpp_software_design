# 基底クラスとコンセプトの共通点を把握する

LSPは継承関係と基底クラスのみを対象としているのではありません。
LSPは動的（実行時）多態や継承に限った話ではなく、静的（コンパイル時）多態やテンプレートにも適用できます。

次の2つのコードの差異はなんでしょうか？

```C++
class Document {
  public:
    // ...
    virtual ~Document() = default;

    virtual void exportToJSON( /* ... */ ) const = 0;
    virtual void seriallize(ByteStream&, /* ... */ ) const = 0;
    // ...
};

void useDocument(Document const& doc) {
  // ...
  doc.exportToJSON( /* ... */ );
  // ...
};
```

```C++
template <typename T>
concept Document = 
  requires(T t, ByteStream b) {
    t.exportToJSON(b);
    e.serialize(b, /* ... */);
  };

template<Document T>
void useDocument(T const& doc) {
  // ...
  doc.exportToJSON( /* ... */ );
  // ...
}
```   

先のコードは**動的多態性**(dynamic polymorphism)を使っています。後者は**静的多態性**(static polymorphism)を使っています。

2つのコードは意味的にはどう異なるでしょうか？
先のコードでは、useDocument()関数はDocumentの派生クラスのみを対象とします。これはDoucment抽象化が規定する内容に従ったクラスのみを対象にすると言えます。
後のコードでは、useDoucment()関数はDocument conceptを実装したクラスのみを対象とします。Document抽象化が期待する内容に従ったクラスのみを対象にします。
両コードとも、Document抽象化が期待する内容に従ったクラスのみを対象にするのです。動的多態性と静的多態性の違いはありますが、意味的にはにています。

基底クラスもconceptも要件を表現するもので、いずれも期待する動作を公式に記述するものであり、コール側に対する要件を表現する方法です。そのため、conceptは基底クラスの静的な透過機能と言えます。この観点に立てば、LSPはテンプレートも対象にすると考えてもよいでしょう。

コンセプトはLSP抽象化、すなわち一連の要件や期待する内容を表現するため、**インターフェース分離の原則**の対象にもなる点に注意してましょう。要件定義を基底クラスという形で関心の分離を図るべきなのと同様に（この基底クラスを"interface"クラスとでも呼びましょう）、concept定義時には関心の分離すべきです。

標準ライブラリのイテレータはその構築に、他のイテレータを用いることで関心を分離し、要件レベルで選択を可能にしています。

```C++
template <typename I>
concept input_or_output_iterator = 
  /* ... */;

template <typename I>
concept input_iterator =
  std::input_or_output_iterator<I> && 
  /* ... */;

template <typename I>
concept forward_iterator = 
  std::input_iterator<I> &&
  /* ... */;
``` 

## まとめ
- 動的多態性、静的多態性、いずれにもLSPを適用する。
- コンセプトは（C++20のコンセプトもC++20以前のテンプレート引数名も）、基底クラス相当機能を静的に表現したものと捉える。
- テンプレートを使用する場合、コンセプトが期待する動作を満たすようにする。
- コンセプトが期待する内容を明確化する。（特にC++20以前のテンプレート引数名の場合）
