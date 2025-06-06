#pragma once

#include <Columns/IColumn.h>

namespace DB
{

class ColumnArray;
class ColumnTuple;

/** Column, that stores a nested Array(Tuple(key, value)) column.
 */
class ColumnMap final : public COWHelper<IColumnHelper<ColumnMap>, ColumnMap>
{
private:
    friend class COWHelper<IColumnHelper<ColumnMap>, ColumnMap>;

    WrappedPtr nested;

    explicit ColumnMap(MutableColumnPtr && nested_);

    ColumnMap(const ColumnMap &) = default;

public:
    /** Create immutable column using immutable arguments. This arguments may be shared with other columns.
      * Use IColumn::mutate in order to make mutable column and mutate shared nested columns.
      */
    using Base = COWHelper<IColumnHelper<ColumnMap>, ColumnMap>;

    static Ptr create(const ColumnPtr & keys, const ColumnPtr & values, const ColumnPtr & offsets);

    static Ptr create(const ColumnPtr & column) { return ColumnMap::create(column->assumeMutable()); }
    static Ptr create(ColumnPtr && arg) { return create(arg); }

    template <typename ... Args>
    requires (IsMutableColumns<Args ...>::value)
    static MutablePtr create(Args &&... args) { return Base::create(std::forward<Args>(args)...); }

    std::string getName() const override;
    const char * getFamilyName() const override { return "Map"; }
    TypeIndex getDataType() const override { return TypeIndex::Map; }

    MutableColumnPtr cloneEmpty() const override;
    MutableColumnPtr cloneResized(size_t size) const override;

    size_t size() const override { return nested->size(); }

    Field operator[](size_t n) const override;
    void get(size_t n, Field & res) const override;
    std::pair<String, DataTypePtr> getValueNameAndType(size_t n) const override;

    bool isDefaultAt(size_t n) const override;
    StringRef getDataAt(size_t n) const override;
    void insertData(const char * pos, size_t length) override;
    void insert(const Field & x) override;
    bool tryInsert(const Field & x) override;
    void insertDefault() override;
    void popBack(size_t n) override;
    StringRef serializeValueIntoArena(size_t n, Arena & arena, char const *& begin) const override;
    char * serializeValueIntoMemory(size_t n, char * memory) const override;
    const char * deserializeAndInsertFromArena(const char * pos) override;
    const char * skipSerializedInArena(const char * pos) const override;
    void updateHashWithValue(size_t n, SipHash & hash) const override;
    WeakHash32 getWeakHash32() const override;
    void updateHashFast(SipHash & hash) const override;

#if !defined(DEBUG_OR_SANITIZER_BUILD)
    void insertFrom(const IColumn & src_, size_t n) override;
    void insertManyFrom(const IColumn & src, size_t position, size_t length) override;
    void insertRangeFrom(const IColumn & src, size_t start, size_t length) override;
#else
    void doInsertFrom(const IColumn & src_, size_t n) override;
    void doInsertManyFrom(const IColumn & src, size_t position, size_t length) override;
    void doInsertRangeFrom(const IColumn & src, size_t start, size_t length) override;
#endif

    ColumnPtr filter(const Filter & filt, ssize_t result_size_hint) const override;
    void expand(const Filter & mask, bool inverted) override;
    ColumnPtr permute(const Permutation & perm, size_t limit) const override;
    ColumnPtr index(const IColumn & indexes, size_t limit) const override;
    ColumnPtr replicate(const Offsets & offsets) const override;
    MutableColumns scatter(ColumnIndex num_columns, const Selector & selector) const override;
#if !defined(DEBUG_OR_SANITIZER_BUILD)
    int compareAt(size_t n, size_t m, const IColumn & rhs, int nan_direction_hint) const override;
#else
    int doCompareAt(size_t n, size_t m, const IColumn & rhs, int nan_direction_hint) const override;
#endif
    void getExtremes(Field & min, Field & max) const override;
    void getPermutation(IColumn::PermutationSortDirection direction, IColumn::PermutationSortStability stability,
                        size_t limit, int nan_direction_hint, IColumn::Permutation & res) const override;
    void updatePermutation(IColumn::PermutationSortDirection direction, IColumn::PermutationSortStability stability,
                        size_t limit, int nan_direction_hint, IColumn::Permutation & res, EqualRanges & equal_ranges) const override;
    void reserve(size_t n) override;
    size_t capacity() const override;
    void prepareForSquashing(const Columns & source_columns, size_t factor) override;
    void shrinkToFit() override;
    void ensureOwnership() override;
    size_t byteSize() const override;
    size_t byteSizeAt(size_t n) const override;
    size_t allocatedBytes() const override;
    void protect() override;
    ColumnCheckpointPtr getCheckpoint() const override;
    void updateCheckpoint(ColumnCheckpoint & checkpoint) const override;
    void rollback(const ColumnCheckpoint & checkpoint) override;
    void forEachMutableSubcolumn(MutableColumnCallback callback) override;
    void forEachMutableSubcolumnRecursively(RecursiveMutableColumnCallback callback) override;
    void forEachSubcolumn(ColumnCallback callback) const override;
    void forEachSubcolumnRecursively(RecursiveColumnCallback callback) const override;
    bool structureEquals(const IColumn & rhs) const override;
    void finalize() override { nested->finalize(); }
    bool isFinalized() const override { return nested->isFinalized(); }

    const ColumnArray & getNestedColumn() const;
    ColumnArray & getNestedColumn();

    const ColumnPtr & getNestedColumnPtr() const { return nested; }
    ColumnPtr & getNestedColumnPtr() { return nested; }

    const ColumnTuple & getNestedData() const;
    ColumnTuple & getNestedData();

    ColumnPtr compress(bool force_compression) const override;

    bool hasDynamicStructure() const override { return nested->hasDynamicStructure(); }
    bool dynamicStructureEquals(const IColumn & rhs) const override;
    void takeDynamicStructureFromSourceColumns(const Columns & source_columns) override;
};

}
