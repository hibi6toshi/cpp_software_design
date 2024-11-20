# インターフェースを分離し人工的関係性を排除する

## インターフェースを分離し関心の分離を図る
Documentクラスを考える。

```C++
class Document {
  public: 
    // ...
    virtual ~Document() = default;

    virtual void exportToJSON( /*...*/ ) const = 0;
    virtual void serialize(ByteStream&, /*...*/ ) const = 0;
    // ...
};
```

次のコードを考える
```C++
viod exportDocument(Document const& doc) {
  // ...
  doc.exportToJSON( /* pass necessary arguments */);
  // ...
}
```

上記のexportDocument()関数は渡されたドキュメントをJSON形式へエクスポートするだけで、シリアライゼーションやDocumentが備える他のアスペクトに**一切関知していません**。それでいながらDocumentの直行する多くのアスペクトを一まとめにするインターフェース定義のせいで、exportDocument()関数はJSONエクスポート以外のものにも依存してしまうのです。

この関係性の原因は**インターフェース分離の原則**からの逸脱にある。
本書の例で言えば、JSONエクスポートとシリアライゼーションの直行するアスペクトは分離すべきインターフェースです。
```C++
class JSONExportable {
  public:
    // ...
    virtual ~JSONExportable() = default;

    virtual void exportToJSON(/*...*/) const = 0;
    // ...
};

class Serializable {
  public:
    // ...
    virtual ~Serializable() = default;

    virtual void serialize(ByteStream& bs, /*...*/) const = 0;
    // ...
};

class Document 
  : public JSONExportable
  , public Serializable {
  public:
    // ...
    void exportToJSON(/*...*/) const override {
        // ...
    }

    void serialize(ByteStream& bs, /*...*/) const override {
        // ...
    }
    // ...
};
```
上記の関心の分離により、依存関係を実際に必要な関数セットにまで最小化できる。
```C++
void exportDocument(JSONExportable const& exportable) {
    // ...
    exportable.exportToJSON(/* pass necessary arguments */);
    // ...
}
```
上記のexportDocument()関数は分離されたJSONExportableインターフェースにのみ依存しており、シリアライゼーションには依存しなくなりました。ByteStreamクラスにはもう依存しておらず、疎結合にできた。