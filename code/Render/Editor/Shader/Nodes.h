#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Render/Editor/Shader/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace render
	{

/*! \ingroup Render */
//@{

/*! Absolute value. */
class T_DLLCLASS Abs : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Abs();
};

/*! Add two values. */
class T_DLLCLASS Add : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Add();
};

/*! Arcus cosine. */
class T_DLLCLASS ArcusCos : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ArcusCos();
};

/*! Absolute tangent. */
class T_DLLCLASS ArcusTan : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ArcusTan();
};

/*! Static branch permutation. */
class T_DLLCLASS Branch : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Branch(const std::wstring& parameterName = L"");

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_parameterName;
};

/*! Clamp value to given range. */
class T_DLLCLASS Clamp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Clamp(float min = 0.0f, float max = 1.0f);

	void setMin(float min);

	float getMin() const;

	void setMax(float max);

	float getMax() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_min;
	float m_max;
};

/*! Color constant. */
class T_DLLCLASS Color : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Color(const traktor::Color4f& color = traktor::Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	void setColor(const traktor::Color4f& color);

	const traktor::Color4f& getColor() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	traktor::Color4f m_color;
};

/*! Comment node. */
class T_DLLCLASS Comment : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Comment();
};

/*! Compute output node. */
class T_DLLCLASS ComputeOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ComputeOutput();

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_technique;
};

/*! Conditional selection. */
class T_DLLCLASS Conditional : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Branch
	{
		BrAuto,
		BrStatic,
		BrDynamic
	};

	enum Operator
	{
		CoLess,
		CoLessEqual,
		CoEqual,
		CoNotEqual,
		CoGreater,
		CoGreaterEqual
	};

	Conditional();

	void setBranch(Branch branch);

	Branch getBranch() const;

	void setOperator(Operator op);

	Operator getOperator() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Branch m_branch;
	Operator m_operator;
};

/*! Connected. */
class T_DLLCLASS Connected : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Connected();
};

/*! Cosine. */
class T_DLLCLASS Cos : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Cos();
};

/*! Cross product. */
class T_DLLCLASS Cross : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Cross();
};

/*! Partial derivative. */
class T_DLLCLASS Derivative : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Axis
	{
		DaX,
		DaY
	};

	Derivative();

	Axis getAxis() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Axis m_axis;
};

/*! Discard fragment. */
class T_DLLCLASS Discard : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Operator
	{
		CoLess,
		CoLessEqual,
		CoEqual,
		CoNotEqual,
		CoGreater,
		CoGreaterEqual
	};

	Discard();

	void setOperator(Operator op);

	Operator getOperator() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Operator m_operator;
};

/*! Compute dispatch index. */
class T_DLLCLASS DispatchIndex : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	DispatchIndex();
};

/*! Divide. */
class T_DLLCLASS Div : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Div();
};

/*! Dot product. */
class T_DLLCLASS Dot : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Dot();
};

/*! Exponential. */
class T_DLLCLASS Exp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Exp();
};

/*! Fractional part. */
class T_DLLCLASS Fraction : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Fraction();
};

/*! Fragment position. */
class T_DLLCLASS FragmentPosition : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	FragmentPosition();
};

/*! Emit true if primitive is seen from the front. */
class T_DLLCLASS FrontFace : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	FrontFace();
};

/*! Fetch value from array uniform. */
class T_DLLCLASS IndexedUniform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	IndexedUniform(
		const std::wstring& parameterName = L"",
		ParameterType type = PtScalar,
		UpdateFrequency frequency = UfFrame,
		int32_t length = 0
	);

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;

	void setFrequency(UpdateFrequency frequency);

	UpdateFrequency getFrequency() const;

	void setLength(int32_t length);

	int32_t getLength() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_parameterName;
	ParameterType m_type;
	UpdateFrequency m_frequency;
	int32_t m_length;
};

/*! Fragment named input. */
class T_DLLCLASS InputPort : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	InputPort();

	InputPort(const std::wstring& name, bool connectable, bool optional, bool haveDefaultValue, float defaultValue);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setConnectable(bool connectable);

	bool isConnectable() const;

	void setOptional(bool optional);

	bool isOptional() const;

	void setHaveDefaultValue(bool haveDefaultValue);

	bool haveDefaultValue() const;

	void setDefaultValue(float defaultValue);

	float getDefaultValue() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	bool m_connectable;
	bool m_optional;
	bool m_haveDefaultValue;
	float m_defaultValue;
};

/*! Instance index input. */
class T_DLLCLASS Instance : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Instance();
};

/*! Interpolator, pass value from vertex to pixel shader. */
class T_DLLCLASS Interpolator : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Interpolator();
};

/*! Iterate. */
class T_DLLCLASS Iterate : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Iterate(int32_t from = 0, int32_t to = 0);

	void setFrom(int32_t from);

	int32_t getFrom() const;

	void setTo(int32_t to);

	int32_t getTo() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_from;
	int32_t m_to;
};

/*! Iterate2
 *
 * \tbd Replace all others, Sum, Repeat, Iterate and Iterate2d with fragments using this node.
 */
class T_DLLCLASS Iterate2 : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Iterate2();
};

/*! 2-D iterate. */
class T_DLLCLASS Iterate2d : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Iterate2d(int32_t fromX = 0, int32_t toX = 0, int32_t fromY = 0, int32_t toY = 0);

	void setFromX(int32_t fromX);

	int32_t getFromX() const;

	void setToX(int32_t toX);

	int32_t getToX() const;

	void setFromY(int32_t fromY);

	int32_t getFromY() const;

	void setToY(int32_t toY);

	int32_t getToY() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_fromX;
	int32_t m_toX;
	int32_t m_fromY;
	int32_t m_toY;
};

/*! Length of geometrical vector. */
class T_DLLCLASS Length : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Length();
};

/*! Linear interpolate. */
class T_DLLCLASS Lerp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Lerp();
};

/*! Natural logarithm. */
class T_DLLCLASS Log : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Base
	{
		LbTwo,
		LbTen,
		LbNatural
	};

	Log(Base base = LbTen);

	Base getBase() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Base m_base;
};

/*! Build matrix. */
class T_DLLCLASS MatrixIn : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MatrixIn();
};

/*! Decompose matrix. */
class T_DLLCLASS MatrixOut : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MatrixOut();
};

/*! Maximum of two values. */
class T_DLLCLASS Max : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Max();
};

/*! Minimum of two values. */
class T_DLLCLASS Min : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Min();
};

/*! Mix in scalars into vector. */
class T_DLLCLASS MixIn : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MixIn();
};

/*! Extract scalars from vector. */
class T_DLLCLASS MixOut : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MixOut();
};

/*! Multiply. */
class T_DLLCLASS Mul : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Mul();
};

/*! Multiply then add. */
class T_DLLCLASS MulAdd : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MulAdd();
};

/*! Negate value. */
class T_DLLCLASS Neg : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Neg();
};

/*! Normalize vector. */
class T_DLLCLASS Normalize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Normalize();
};

/*! Shader fragment named output. */
class T_DLLCLASS OutputPort : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	OutputPort(const std::wstring& name = L"");

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
};

/*! Evaluate polynomial. */
class T_DLLCLASS Polynomial : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Polynomial();
};

/*! Raise to power. */
class T_DLLCLASS Pow : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Pow();
};

/*! Pixel output. */
class T_DLLCLASS PixelOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	PixelOutput();

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	void setPriority(uint32_t priority);

	uint32_t getPriority() const;

	void setRenderState(const RenderState& state);

	const RenderState& getRenderState() const;

	void setRegisterCount(uint32_t registerCount);

	uint32_t getRegisterCount() const;

	void setPrecisionHint(PrecisionHint precisionHint);

	PrecisionHint getPrecisionHint() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_technique;
	uint32_t m_priority;
	RenderState m_renderState;
	uint32_t m_registerCount;
	PrecisionHint m_precisionHint;
};

/*! Read [indexed] element from struct buffer. */
class T_DLLCLASS ReadStruct : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ReadStruct();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
};

/*! Reciprocal square root. */
class T_DLLCLASS RecipSqrt : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	RecipSqrt();
};

/*! Reflect vector. */
class T_DLLCLASS Reflect : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Reflect();
};

/*! Conditional based on renderer. */
class T_DLLCLASS Renderer : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Renderer();
};

/*! Repeat until condition. */
class T_DLLCLASS Repeat : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Repeat();
};

/*! Round. */
class T_DLLCLASS Round : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Round();
};

/*! Texture sampler. */
class T_DLLCLASS Sampler : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sampler();

	Sampler(const SamplerState& state);

	void setSamplerState(const SamplerState& state);

	const SamplerState& getSamplerState() const;

	virtual void serialize(ISerializer& s) override final;

private:
	SamplerState m_state;
};

/*! Scalar constant. */
class T_DLLCLASS Scalar : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Scalar(float value = 0.0f);

	void set(float value);

	float get() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_value;
};

/*! Sign. */
class T_DLLCLASS Sign : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sign();
};

/*! Sine. */
class T_DLLCLASS Sin : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sin();
};

/*! Square root of value. */
class T_DLLCLASS Sqrt : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sqrt();
};

/*! Output states. */
class T_DLLCLASS State : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	State();

	void setPriority(uint32_t priority);

	uint32_t getPriority() const;

	void setRenderState(const RenderState& renderState);

	const RenderState& getRenderState() const;

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_priority;
	RenderState m_renderState;
};

/*! Step function. */
class T_DLLCLASS Step : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Step();
};

/*! Define struct. */
class T_DLLCLASS Struct : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	struct NamedElement
	{
		std::wstring name;
		DataType type;

		NamedElement();

		void serialize(ISerializer& s);
	};

	Struct();

	const std::wstring& getParameterName() const;

	const AlignedVector< NamedElement >& getElements() const;

	DataType getElementType(const std::wstring& name) const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_parameterName;
	AlignedVector< NamedElement > m_elements;

};

/*! Subtract. */
class T_DLLCLASS Sub : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sub();
};

/*! Summarize function. */
class T_DLLCLASS Sum : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sum(int32_t from = 0, int32_t to = 0);

	void setFrom(int32_t from);

	int32_t getFrom() const;

	void setTo(int32_t to);

	int32_t getTo() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_from;
	int32_t m_to;
};

/*! Conditional switch. */
class T_DLLCLASS Switch : public Node
{
	T_RTTI_CLASS;

public:
	enum Branch
	{
		BrAuto,
		BrStatic,
		BrDynamic
	};

	Switch();

	void setBranch(Branch branch);

	Branch getBranch() const;

	void addCase(int32_t value);

	const std::vector< int32_t >& getCases() const;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Branch m_branch;
	std::vector< int32_t > m_cases;
	std::vector< InputPin* > m_inputPins;
	OutputPin* m_outputPin;
};

/*! Swizzle elements in a vector. */
class T_DLLCLASS Swizzle : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Swizzle(const std::wstring& swizzle = L"xyzw");

	void set(const std::wstring& swizzle);

	const std::wstring& get() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_swizzle;
};

/*! Tangent. */
class T_DLLCLASS Tan : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Tan();
};

/*! Target size. */
class T_DLLCLASS TargetSize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	TargetSize();
};

/*! Texture constant. */
class T_DLLCLASS Texture : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Texture(const Guid& external = Guid(), ParameterType type = PtTexture2D);

	void setExternal(const Guid& external);

	const Guid& getExternal() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_external;
	ParameterType m_type;
};


/*! Texture size. */
class T_DLLCLASS TextureSize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	TextureSize();
};

/*! Transform vector by matrix. */
class T_DLLCLASS Transform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Transform();
};

/*! Transpose matrix. */
class T_DLLCLASS Transpose : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Transpose();
};

/*! Truncate. */
class T_DLLCLASS Truncate : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Truncate();
};

/*! Type switch. */
class T_DLLCLASS Type : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Type();
};

/*! Fetch parameter value. */
class T_DLLCLASS Uniform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Uniform(
		const std::wstring& parameterName = L"",
		ParameterType type = PtScalar,
		UpdateFrequency frequency = UfFrame
	);

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;

	void setFrequency(UpdateFrequency frequency);

	UpdateFrequency getFrequency() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_parameterName;
	ParameterType m_type;
	UpdateFrequency m_frequency;
};

/*! Variable */
class T_DLLCLASS Variable : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Variable();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	bool isGlobal() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	bool m_global;
};

/*! Vector constant. */
class T_DLLCLASS Vector : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Vector(const Vector4& value = Vector4(0, 0, 0, 0));

	void set(const Vector4& value);

	const Vector4& get() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_value;
};

/*! Fetch value from vertex. */
class T_DLLCLASS VertexInput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	VertexInput(const std::wstring& name = L"", DataUsage usage = DuPosition, DataType type = DtFloat4, int index = 0);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setDataUsage(DataUsage usage);

	DataUsage getDataUsage() const;

	void setDataType(DataType type);

	DataType getDataType() const;

	void setIndex(int32_t index);

	int32_t getIndex() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	DataUsage m_usage;
	DataType m_type;
	int32_t m_index;
};

/*! Output vertex's clip position. */
class T_DLLCLASS VertexOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	VertexOutput();

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	virtual std::wstring getInformation() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_technique;
};

//@}

	}
}
